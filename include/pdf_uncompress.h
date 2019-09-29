#ifndef PDF_UNCOMPRESS_H
#define PDF_UNCOMPRESS_H
#include <stdio.h>
#include <zlib.h>
#include <zconf.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include "pdf_struct.h"
#include "jconfig.h"
#include "jerror.h"
//#include "jmorecfg.h"
#include <jpeglib.h>

#ifdef __cplusplus
extern "C"
{
#endif
    namespace pdf
    {
    class PdfUncompress
    {
    public:
        bool PdfUncompressData(unsigned char *input_src, int src_length, unsigned char *output_dest, size_t &output_length, const std::string &type);
        bool ZilbcompressData(unsigned char *input_src, int src_length, unsigned char *output_dest, size_t &output_length);

    private:
        pdf::CompressType GetType(const std::string &input);
        bool DCTDecodeUncompress(unsigned char *input_src, int src_length, unsigned char *output_dest, size_t &output_length);
    };
    } //namespace pdf

#ifdef __cplusplus
}
#endif
#endif // PDF_UNCOMPRESS_H