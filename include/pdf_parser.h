/*
 * @Author: mikey.yuanjian 
 * @Date: 2019-06-15 15:34:19 
 * @Last Modified by: mikey.yuanjian
 * @Last Modified time: 2019-06-15 19:49:57
 */
#ifndef PDF_PARSER_H
#define PDF_PARSER_H
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>
#include "pdf_basicdata_singleton.h"
#include "pdf_struct.h"
#include "pdf_common_function.h"
#ifdef __cplusplus
extern "C"
{
#endif
    constexpr int kLineSizeObject = 64;
    constexpr int kLineSizeLength = 8;
    constexpr int kLineSizeXref = 21;

    namespace pdf
    {
    class PdfParser
    {
    public:
        PdfParser() : is_end_dict_parser_(false){};
        ~PdfParser(){};

    public:
        //get object content by file offset
        bool ObjectParser(FILE *file_ptr);
        //get indirect object by file pointer
        bool IndirectObjectParser(FILE *file_ptr, int &output_object);
        //dictionary parser
        //if success return true,else false
        bool DictionaryParser(FILE *file_ptr, std::unordered_map<std::string, std::string> &output_map);
        bool DictionaryParser(const std::string &input, std::unordered_map<std::string, std::string> &output_map);
        //get stream by file pointer
        bool StreamParser(FILE *file_ptr, std::string &length_string);
        //get length by length object
        bool GetLengthOfLengthObject(FILE *file_ptr, int &length);
        //get xref table
        bool GetXrefTable(FILE *file_ptr, std::vector<pdf::XrefTable> &output_vector);
        //get startxref
        int GetStartXref(FILE *file_ptr);
        //get trailer
        bool GetTrailer(FILE *file_ptr);

    private:
        //get content () [] / value
        std::string GetStringByFile(FILE *file_ptr);
        std::string GetStringByFile(FILE *file_ptr, char input, char end_input);
        //<<>>
        std::string DictDict(FILE *file_ptr);
        //read line data from end
        std::string ReadLineData(FILE *file_ptr);

    private:
        bool is_end_dict_parser_;
    };
    } // namespace pdf
#ifdef __cplusplus
}
#endif
#endif // PDF_PARSER_H