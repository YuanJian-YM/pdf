#ifndef PDF_CHILE_OBJECT_ANALYSIS_H
#define PDF_CHILE_OBJECT_ANALYSIS_H
#include <stdio.h>
#include <vector>
#include <string.h>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <memory>
#include "pdf_basicdata_singleton.h"
#include "pdf_parser.h"
#include "pdf_common_function.h"
#ifdef __cplusplus
extern "C"
{
#endif
    namespace pdf
    {
    class PdfPageAnalysis
    {
    public:
        PdfPageAnalysis(int number);
        ~PdfPageAnalysis();

    public:
        std::unordered_map<std::string, int> GetFontObject() const;
        std::unordered_map<std::string, int> GetXObjectObject() const;
        std::vector<double> GetMediaBox() const;
        std::string GetDevice() const;
        int GetContentsObject() const;

    private:
        void AnalysisChild();
        //parser xobject
        std::unordered_map<std::string, int> ParserXobject(const std::string &input);

    private:
        std::unordered_map<std::string, int> font_object_;
        std::unordered_map<std::string, int> xobject_map_;
        std::vector<double> mediabox_;
        std::string colorspace_;
        FILE *file_ptr_;
        int contents_obj_number_;
    };
    typedef std::shared_ptr<PdfPageAnalysis> PdfPageAnalysisPtr;
    } // namespace pdf

#ifdef __cplusplus
}
#endif
#endif // PDF_CHILE_OBJECT_ANALYSIS_H