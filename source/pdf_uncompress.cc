#include "pdf_uncompress.h"
namespace pdf
{
bool PdfUncompress::PdfUncompressData(unsigned char *input_src,
                                      int src_length,
                                      unsigned char *output_dest,
                                      size_t &output_length,
                                      const std::string &type)
{
    int status;
    switch (GetType(type))
    {
    case CompressType::kFlateDecode:
        status = uncompress(output_dest, &output_length, input_src, src_length);
        if (status == Z_OK)
            return true;
        else
            return false;
        break;
    case CompressType::kDCTDecode:
        if (DCTDecodeUncompress(
                input_src, src_length, output_dest, output_length))
            return true;
        else
            return false;
        break;
    default:
        return false;
        break;
    }
}

bool PdfUncompress::ZilbcompressData(unsigned char *input_src,
                                     int src_length,
                                     unsigned char *output_dest,
                                     size_t &output_length)
{
    int status;
    status = compress(output_dest, &output_length, input_src, src_length);
    if (status == Z_OK)
        return true;
    else
        return false;
}
pdf::CompressType
PdfUncompress::GetType(const std::string &input)
{
    std::stringstream stream(input);
    std::string value;
    stream >> value;
    if (value == "ASCIIHexDecode")
        return CompressType::kASCIIHexDecode;
    else if (value == "ASCII85Decode")
        return CompressType::kASCII85Decode;
    else if (value == "LZWDecode")
        return CompressType::kLZWDecode;
    else if (value == "FlateDecode")
        return CompressType::kFlateDecode;
    else if (value == "RunLengthDecode")
        return CompressType::kRunLengthDecode;
    else if (value == "CCITTFaxDecode")
        return CompressType::kCCITTFaxDecode;
    else if (value == "JBIG2Decode")
        return CompressType::kJBIG2Decode;
    else if (value == "DCTDecode")
        return CompressType::kDCTDecode;
    else if (value == "JPXDecode")
        return CompressType::kJPXDecode;
    else if (value == "Crypt")
        return CompressType::kCrypt;
}
bool PdfUncompress::DCTDecodeUncompress(unsigned char *input_src,
                                        int src_length,
                                        unsigned char *output_dest,
                                        size_t &output_length)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerror;
    cinfo.err = jpeg_std_error(&jerror);
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, input_src, src_length);
    jpeg_read_header(&cinfo, TRUE);
    if (jpeg_start_decompress(&cinfo) == false)
        return false;
    int width = cinfo.image_width;
    int height = cinfo.image_height;
    int colorspace = cinfo.out_color_components;
    output_length = width * height * colorspace;
    unsigned char *temp_ptr = new unsigned char[output_length];
    unsigned char *tptr = temp_ptr;

    int numlines = 0;
    while (cinfo.output_scanline < cinfo.output_height)
    {
        numlines = jpeg_read_scanlines(&cinfo, &tptr, 1);
        tptr += numlines * width * colorspace;
    }
    memcpy(output_dest, temp_ptr, output_length);
    delete[] temp_ptr;
    temp_ptr = nullptr;
    return true;
}

} // namespace pdf