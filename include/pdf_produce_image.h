#ifndef PDF_PRODUCE_IMAGE_H
#define PDF_PRODUCE_IMAGE_H
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <fstream>
#include "pdf_struct.h"
#include "pdf_uncompress.h"
#ifdef __cplusplus
extern "C"
{
#endif
    namespace pdf
    {
    constexpr int kBufferSize = 2048 * 2048;

    static void WriteData(std::ofstream *ofs, int input)
    {
        char data[4];
        data[0] = input >> 24;
        data[1] = input >> 16;
        data[2] = input >> 8;
        data[3] = input;
        ofs->write(data, 4);
    };
    static void putchunk(std::ofstream *ofs, char *tag, unsigned char *data, int size)
    {
        unsigned int sum;
        WriteData(ofs, size);
        ofs->write(tag, 4);
        ofs->write((char *)data, size);
        sum = crc32(0, NULL, 0);
        sum = crc32(sum, (unsigned char *)tag, 4);
        sum = crc32(sum, data, size);
        WriteData(ofs, sum);
    };
    static void BigData(unsigned char *buf, unsigned int input)
    {
        buf[0] = (input >> 24) & 0xff;
        buf[1] = (input >> 16) & 0xff;
        buf[2] = (input >> 8) & 0xff;
        buf[3] = (input)&0xff;
    };
    static void WritePngHeader(std::ofstream *ofs, Image image)
    {
        static const unsigned char pngsig[8] = {137, 80, 78, 71, 13, 10, 26, 10};
        unsigned char head[13];
        int alpha = image.alpha;
        int color;

        if (image.colorspace == 1 && image.alpha)
            alpha = 0;

        switch (image.colorspace - alpha)
        {
        case 1:
            color = (alpha ? 4 : 0);
            break;
        case 3:
            color = (alpha ? 6 : 2);
            break;
        }

        BigData(head + 0, image.width);
        BigData(head + 4, image.height);
        head[8] = 8;
        head[9] = color;
        head[10] = 0;
        head[11] = 0;
        head[12] = 0;
        ofs->write((char *)pngsig, 8);
        putchunk(ofs, (char *)"IHDR", head, 13);
        putchunk(ofs, (char *)"iCCP", (unsigned char *)"Shannon.AI", 10);
    };
    static bool WriteDataToPng(char *cSource, const std::string &outfile, const Image &image)
    {
        unsigned char *source = (unsigned char *)cSource;
        //std::cout<<cSource<<std::endl;
        std::ofstream ofs;

        ofs.open(outfile, std::ios::binary | std::ios::out);
        if (ofs.fail())
        {
            std::cout << "can't open file" << std::endl;
            return false;
        }
        unsigned long finallLen = image.height + image.height * image.width * image.colorspace;
        unsigned char *finallData = new unsigned char[finallLen];
        unsigned char *finall = finallData;

        for (int i = 0; i < image.height; ++i)
        {
            *finall++ = 0;
            for (int j = 0; j < image.width * image.colorspace; ++j)
            {
                *finall = *source;
                source++;
                finall++;
            }
        }
        unsigned char *destData = new unsigned char[kBufferSize];
        size_t destlen = kBufferSize;
        pdf::PdfUncompress pdfuncompress;
        if (pdfuncompress.ZilbcompressData(finallData, finallLen, destData, destlen))
        {
            WritePngHeader(&ofs, image);
            putchunk(&ofs, (char *)"IDAT", destData, destlen);
        }
        else
        {
            return false;
        }
        ofs.close();
        return true;
    };
    static void ChangeImageSize(char *cSource, char *dest, Image image, int width, int height, int colorspace)
    {
        unsigned char *srcstm = (unsigned char *)cSource;
        double xscale = (double)image.width / width;
        double yscale = (double)image.height / height;
        for (int i = 0; i < height; ++i)
        {
            for (int j = 0; j < width; ++j)
            {
                double x = j * xscale;
                double y = i * yscale;
                int indexX = floor(x);
                double u = x - indexX;
                int indexY = floor(y);
                double v = y - indexY;

                int index0 = (indexY * image.width + indexX) * colorspace;
                int index1 = (indexY * image.width + indexX + 1) * colorspace;
                int index2 = ((indexY + 1) * image.height + indexX) * colorspace;
                int index3 = ((indexY + 1) * image.height + indexX + 1) * colorspace;

                for (int k = 0; k < colorspace; ++k)
                {
                    *dest++ = (1 - u) * (1 - v) * srcstm[index0++] + u * (1 - v) * srcstm[index1++] + (1 - u) * v * srcstm[index2++] + u * v * srcstm[index3++];
                }
            }
        }
    };
    } // namespace pdf
#ifdef __cplusplus
}
#endif
#endif // PDF_RENDER_IMAGE_H