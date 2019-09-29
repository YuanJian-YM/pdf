#ifndef _PDF_PARSER_CONTENTS_H
#define _PDF_PARSER_CONTENTS_H
#include <stdio.h>
#include <string.h>
#include <functional>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <math.h>
#include "pdf_chile_object_analysis.h"
#include "pdf_image_text.h"
#include "pdf_struct.h"
#include "pdf_uncompress.h"
#include "pdf_produce_image.h"
constexpr int kUncompressSize = 2048 * 2048;
#ifdef __cplusplus
extern "C"
{
#endif
    namespace pdf
    {
    class PdfParserContents
    {
    public:
        PdfParserContents(int pagenumber);
        ~PdfParserContents();

    public:
        pdf::PdfImagePtr LoadImageData()
        {
            return image_ptr_;
        }
        pdf::PdfTextPtr LoadTextData()
        {
            return text_ptr_;
        }
        std::vector<double> GetMediaBox() const
        {
            return child_page_ptr_->GetMediaBox();
        }
        std::string GetPdfPageColorspace() const
        {
            return child_page_ptr_->GetDevice();
        }

    public:
        void ParserContents(char *input, int length);

    public:
        void DoOther(std::string){};
        void DoRG(std::string input);
        void Dow(std::string input);
        void Dol(std::string input);
        void DoS(std::string input);
        void Dom(std::string input);
        void DoRe(std::string input);
        void DoCm(std::string input);
        void DoDo(std::string input);
        void DoRg(std::string input);
        void DoBT(std::string input);
        void DoTm(std::string input);
        void DoTd(std::string input);
        void DoTD(std::string input);
        void DoTf(std::string input);
        void DoTj(std::string input);
        void DoTJ(std::string input);
        void DoET(std::string input);

    private:
        std::vector<pdf::TjData> ParserTj(std::string input);
        //fill image
        void FillImage(const std::unordered_map<std::string, std::string> &input, ImageData &image_data);
        //fill smask
        void FillImageSmask(const std::unordered_map<std::string, std::string> &input, ImageData::SMask &image_data);
        //deal font data
        void DealFontData();
        //deal content object
        void DealContentsData();

    private:
        std::unordered_map<std::string, std::function<void(std::string)>> data_find_map_;
        PdfImagePtr image_ptr_;
        PdfTextPtr text_ptr_;
        PdfPageAnalysisPtr child_page_ptr_;
        //image cm
        std::vector<double> positon_vec_;
        FILE *file_ptr_;
        pdf::WordData *word_ptr_;
        std::vector<double> color_vec_;
        pdf::LineData *line_ptr_;
    };
    typedef std::shared_ptr<PdfParserContents> pdf_constents_ptr;
    } // namespace pdf

#ifdef __cplusplus
}
#endif
#endif // _PDF_PARSER_CONTENTS_H
