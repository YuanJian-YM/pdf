#ifndef PDF_IMAGE_TEXT_H
#define PDF_IMAGE_TEXT_H
#include <stdio.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <math.h>
#include "pdf_struct.h"
#include "pdf_common_function.h"
#include "pdf_cmap.h"
#ifdef __cplusplus
extern "C"
{
#endif
    namespace pdf
    {
    enum
    {
        UTFmax = 4,         /* maximum bytes per rune */
        Runesync = 0x80,    /* cannot represent part of a UTF sequence (<) */
        Runeself = 0x80,    /* rune and UTF sequences are the same (<) */
        Runeerror = 0xFFFD, /* decoding error in UTF */
        Runemax = 0x10FFFF, /* maximum rune value */
    };
    enum
    {
        Bit1 = 7,
        Bitx = 6,
        Bit2 = 5,
        Bit3 = 4,
        Bit4 = 3,
        Bit5 = 2,

        T1 = ((1 << (Bit1 + 1)) - 1) ^ 0xFF, /* 0000 0000 */
        Tx = ((1 << (Bitx + 1)) - 1) ^ 0xFF, /* 1000 0000 */
        T2 = ((1 << (Bit2 + 1)) - 1) ^ 0xFF, /* 1100 0000 */
        T3 = ((1 << (Bit3 + 1)) - 1) ^ 0xFF, /* 1110 0000 */
        T4 = ((1 << (Bit4 + 1)) - 1) ^ 0xFF, /* 1111 0000 */
        T5 = ((1 << (Bit5 + 1)) - 1) ^ 0xFF, /* 1111 1000 */

        Rune1 = (1 << (Bit1 + 0 * Bitx)) - 1, /* 0000 0000 0111 1111 */
        Rune2 = (1 << (Bit2 + 1 * Bitx)) - 1, /* 0000 0111 1111 1111 */
        Rune3 = (1 << (Bit3 + 2 * Bitx)) - 1, /* 1111 1111 1111 1111 */
        Rune4 = (1 << (Bit4 + 3 * Bitx)) - 1, /* 0001 1111 1111 1111 1111 1111 */

        Maskx = (1 << Bitx) - 1, /* 0011 1111 */
        Testx = Maskx ^ 0xFF,    /* 1100 0000 */

        Bad = Runeerror,
    };

    class PdfImage
    {
    public:
        inline std::vector<pdf::ImageData> GetImageData() const
        {
            return image_data_vec_;
        };
        inline void SetImageData(const std::vector<pdf::ImageData> &input)
        {
            image_data_vec_ = input;
        };
        static void DecodeStream(pdf::ImageData &image);
        static void FuseSmaskImage(pdf::ImageData &image);
        static void ScaleImage(int width, int height, const pdf::ImageData &image, int colorspace, unsigned char *deststm);

    private:
        std::vector<pdf::ImageData> image_data_vec_;
    };
    typedef std::shared_ptr<PdfImage> PdfImagePtr;
    class PdfText
    {
    public:
        inline std::vector<pdf::WordData> GetTextData() const
        {
            return text_data_vec_;
        }
        inline void SetTextData(const std::vector<pdf::WordData> &input)
        {
            text_data_vec_ = input;
        }
        inline std::unordered_map<std::string, pdf::FontData> GetFontData()
        {
            return font_data_;
        }
        inline void SetFontData(const std::unordered_map<std::string, pdf::FontData> &input)
        {
            font_data_ = input;
        }
        inline void SetLinedata(const pdf::LineData &input)
        {
            linedata_ = input;
        }
        inline pdf::LineData GetLineData() const
        {
            return linedata_;
        }
        static void ConvertString(pdf::PDFCmapPtr unicode_ptr, const std::string &input_str, std::string &dest);
        static int RunToChar(char *str, int rune);
        static int FilterChar(int ch);
        static std::string TransWord(const string &input);
        static int HexToNum(char input);

    private:
        std::vector<pdf::WordData> text_data_vec_;
        std::unordered_map<std::string, pdf::FontData> font_data_;
        pdf::LineData linedata_;
    };
    typedef std::shared_ptr<PdfText> PdfTextPtr;
    } //namespace pdf
#ifdef __cplusplus
}
#endif
#endif // PDF_IMAGE_TEXT_H