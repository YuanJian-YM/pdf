#ifndef _PDF_BASE_INIT_H
#define _PDF_BASE_INIT_H
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <unistd.h>
#include "pdf_parser.h"
#include "pdf_struct.h"
#include "pdf_basicdata_singleton.h"
#include "pdf_common_function.h"
#ifdef __cplusplus
extern "C"
{
#endif
    namespace pdf
    {
    class PdfBaseInit
    {
    public:
        PdfBaseInit(std::string input_file, std::string out_file);
        void PdfClose();
        void Init();

    private:
        void GetXrefTable(pdf::PdfParser parser);
        void GetTrailerAndStartXref(pdf::PdfParser parser);
        void GetRootObj();
        void GetKids(pdf::PdfParser parser);
        std::vector<int> AnalysisKids(const std::string &input);

    private:
        FILE *file_ptr_;
        int xref_offset_;
        int root_number;
    };
    } //namespace pdf
#ifdef __cplusplus
}
#endif
#endif // _PDF_BASE_INIT_H