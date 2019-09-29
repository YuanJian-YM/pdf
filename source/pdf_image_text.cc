#include "pdf_image_text.h"
namespace pdf
{
void PdfImage::DecodeStream(pdf::ImageData &image)
{
    int add[32];
    int mul[32];
    int colorspace = pdf::GetColorspaceBydevice(image.smask_data.colorspace);
    std::string decode = image.smask_data.decode;
    for (auto iter = decode.begin(); iter != decode.end();)
    {
        if (*iter == '[' || *iter == ']')
            iter = decode.erase(iter);
        else
            ++iter;
    }
    std::vector<double> decode_vec(64, 0);
    std::stringstream stream(decode);
    int index = 0;
    while (stream >> decode)
        decode_vec[index++] = atoi(decode.c_str());
    for (int i = 0; i < colorspace; ++i)
    {
        int min = decode_vec[i * 2] * 255;
        int max = decode_vec[i * 2 + 1] * 255;
        add[i] = min;
        mul[i] = max - min;
    }
    unsigned char *buf = (unsigned char *)image.smask_data.iamge_str;

    for (int i = 0; i < image.smask_data.height; ++i)
    {
        for (int j = 0; j < image.smask_data.width; ++j)
        {
            for (int k = 0; k < colorspace; ++k)
            {
                int x = *buf * mul[k] + 128;
                x += x >> 8;
                x >>= 8;
                int value = add[k] + x;
                *buf++ = value > 0 ? (value < 255 ? value : 255) : 0;
            }
        }
    }
}
void PdfImage::FuseSmaskImage(pdf::ImageData &image)
{
    DecodeStream(image);
    int maskcolor = pdf::GetColorspaceBydevice(image.smask_data.colorspace);
    int colorspace = pdf::GetColorspaceBydevice(image.colorspace);
    if (maskcolor == 1)
    {
        unsigned char *maskbuf = (unsigned char *)image.smask_data.iamge_str;
        unsigned char *buf = (unsigned char *)image.iamge_str;
        for (int i = 0; i < image.height; ++i)
        {
            for (int j = 0; j < image.width; ++j)
            {
                unsigned int ms = *maskbuf++;
                ms += (ms >> 7);
                if (ms == 0)
                {
                    for (int k = 0; k < colorspace; ++k)
                    {
                        *buf++ = 0xFF;
                    }
                }
                else if (ms == 256)
                {
                    buf += colorspace;
                }
                else
                {
                    unsigned char r = *buf;
                    unsigned char g = *(buf + 1);
                    unsigned char b = *(buf + 2);
                    unsigned int s0 = 0xFF000000;
                    s0 |= (unsigned int)b << 16;
                    s0 |= (unsigned int)g << 8;
                    s0 |= (unsigned int)r;
                    unsigned int mask = 0x00FF00FF;
                    unsigned int d0 = 0xFFFFFFFF;
                    unsigned int d1 = d0 >> 8;
                    unsigned int s1 = s0 >> 8;
                    d0 &= mask;
                    d1 &= mask;
                    s0 &= mask;
                    s1 &= mask;
                    d0 = (((d0 << 8) + (s0 - d0) * ms) >> 8) & mask;
                    d1 = ((d1 << 8) + (s1 - d1) * ms) & ~mask;
                    d0 |= d1;
                    unsigned char *rr = (unsigned char *)&d0;
                    for (int i = 0; i < colorspace; ++i)
                    {
                        *buf++ = *rr++;
                    }
                }
            }
        }
    }
}
void PdfImage::ScaleImage(int width, int height, const pdf::ImageData &image, int colorspace, unsigned char *deststm)
{
    unsigned char *srcstm = (unsigned char *)image.iamge_str;
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
            int index2 = ((indexY + 1) * image.width + indexX) * colorspace;
            int index3 = ((indexY + 1) * image.width + indexX + 1) * colorspace;

            for (int k = 0; k < colorspace; ++k)
            {
                *deststm++ = (1 - u) * (1 - v) * srcstm[index0++] + u * (1 - v) * srcstm[index1++] + (1 - u) * v * srcstm[index2++] + u * v * srcstm[index3++];
            }
        }
    }
}
void PdfText::ConvertString(pdf::PDFCmapPtr unicode_ptr, const std::string &input_str, std::string &dest)
{
    for (std::string::const_iterator ch = input_str.begin(); ch != input_str.end(); ++ch)
    {
        int out[8];
        pdf::lookupCmap(unicode_ptr, *ch, out);
        if (out[0] != -1)
        {
            char utf[10];
            int len = RunToChar(utf, FilterChar(out[0]));
            for (int i = 0; i < len; ++i)
            {
                dest.push_back(utf[i]);
            }
        }
    }
}
int PdfText::RunToChar(char *str, int rune)
{
    unsigned int c = (unsigned int)rune;
    /*
        * one character sequence
        *	00000-0007F => 00-7F
        */
    if (c <= Rune1)
    {
        str[0] = c;
        return 1;
    }
    /*
        * two character sequence
        *	0080-07FF => T2 Tx
        */
    if (c <= Rune2)
    {
        str[0] = T2 | (c >> 1 * Bitx);
        str[1] = Tx | (c & Maskx);
        return 2;
    }
    /*
        * If the Rune is out of range, convert it to the error rune.
        * Do this test here because the error rune encodes to three bytes.
        * Doing it earlier would duplicate work, since an out of range
        * Rune wouldn't have fit in one or two bytes.
        */
    if (c > Runemax)
        c = Runeerror;
    /*
        * three character sequence
        *	0800-FFFF => T3 Tx Tx
        */
    if (c <= Rune3)
    {
        str[0] = T3 | (c >> 2 * Bitx);
        str[1] = Tx | ((c >> 1 * Bitx) & Maskx);
        str[2] = Tx | (c & Maskx);
        return 3;
    }
    /*
        * four character sequence (21-bit value)
        *	10000-1FFFFF => T4 Tx Tx Tx
        */
    str[0] = T4 | (c >> 3 * Bitx);
    str[1] = Tx | ((c >> 2 * Bitx) & Maskx);
    str[2] = Tx | ((c >> 1 * Bitx) & Maskx);
    str[3] = Tx | (c & Maskx);
    return 4;
}
int PdfText::FilterChar(int ch)
{
    switch (ch)
    {
    case 0x0009: /* tab */
    case 0x0020: /* space */
    case 0x00A0: /* no-break space */
    case 0x1680: /* ogham space mark */
    case 0x180E: /* mongolian vowel separator */
    case 0x2000: /* en quad */
    case 0x2001: /* em quad */
    case 0x2002: /* en space */
    case 0x2003: /* em space */
    case 0x2004: /* three-per-em space */
    case 0x2005: /* four-per-em space */
    case 0x2006: /* six-per-em space */
    case 0x2007: /* figure space */
    case 0x2008: /* punctuation space */
    case 0x2009: /* thin space */
    case 0x200A: /* hair space */
    case 0x202F: /* narrow no-break space */
    case 0x205F: /* medium mathematical space */
    case 0x3000: /* ideographic space */
        ch = ' ';
    }
    return ch;
}
std::string PdfText::TransWord(const string &input)
{
    std::string dest;
    bool needAdd = true;
    if (input.size() % 2 == 0)
    {
        needAdd = false;
    }
    for (int i = 0; i < (int)input.size(); i += 2)
    {
        char ch = 0;
        if (i < input.size())
            ch += HexToNum(input[i]) * 16;
        if ((i + 1) < input.size())
            ch += HexToNum(input[i + 1]);
        dest.push_back(ch);
    }
    if (needAdd)
    {
        char ch = 0;
        if ((input.size() - 2) < input.size())
            ch += HexToNum(input[input.size() - 2]) * 16;
        dest.push_back(ch);
    }
    return dest;
}
int PdfText::HexToNum(char input)
{
    char dCh = 0;
    if (input >= '0' && input <= '9')
    {
        dCh = input - '0';
    }
    else if (input >= 'A' && input <= 'F')
    {
        dCh = (input - 'A') + 10;
    }
    else if (input >= 'a' && input <= 'f')
    {
        dCh = (input - 'a') + 10;
    }
    else
    {
        std::cout << "incorrect hex char!" << std::endl;
        return 0;
    }
    return dCh;
}
} //namespace pdf