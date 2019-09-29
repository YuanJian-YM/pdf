#ifndef PDF_PAGE_H
#define PDF_PAGE_H
#include <stdio.h>
#include <vector>
#include <string.h>
#include "pdf_parser_contents.h"
#include "pdf_image_text.h"
#include "pdf_produce_image.h"
#include "pdf_struct.h"
#include "pdf_basicdata_singleton.h"
#include "pdf_cmap.h"
#include <ft2build.h>
#include <freetype/ftglyph.h>
#include <freetype/tttables.h>
#include FT_FREETYPE_H
#ifdef __cplusplus
extern "C"
{
#endif
    namespace pdf
    {
    class PdfPage
    {
    public:
        PdfPage(int pagenumber);
        ~PdfPage();

    public:
        //get pdf sum
        int GetPageSum() const;
        //get mediabox
        std::vector<double> GetMediaBox() const;
        //get text
        std::string GetText();
        //save image to png
        bool SaveImageToPng();
        //render png
        bool RenderPdfToPng();

    private:
        void GetCmapPtr(pdf::PdfTextPtr input_ptr);
        bool RenderImageData(int colorspace, int page_width, int page_height, const pdf::PdfImagePtr &image_ptr, char *render_data);
        bool RenderTextData(int colorspace, int page_width, int page_height, const pdf::PdfTextPtr &text_ptr, char *render_data);
        bool RenderLineData(int colorspace, int page_width, int page_height, const pdf::PdfTextPtr &text_ptr, char *render_data);
        bool ChooseFace(FT_Face &face, const pdf::FontData &fontdata);
        FT_CharMap SelectType1Cmap(FT_Face face);
        FT_CharMap SelectTrueTypeCmap(FT_Face face, int symbolic);
        std::vector<unsigned int> GetCidToGid(FT_Face face);
        std::vector<int> TransHexToNum(const WordData &input, int index);
        void ChangeFontImageData(unsigned char *src, std::vector<double> color, int height, int width, int colorspace, unsigned char *dest);

    private:
        pdf_constents_ptr pdf_contents_ptr_;
        std::unordered_map<std::string, pdf::PDFCmapPtr> cmap_map_;
        int page_number_;

        char *render_data_;
    };
    } // namespace pdf
#ifdef __cplusplus
}
#endif
#endif // PDF_PAGE_H