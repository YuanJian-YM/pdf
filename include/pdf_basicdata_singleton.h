#ifndef _PDF_BASICDATA_SINGLETON_H
#define _PDF_BASICDATA_SINGLETON_H
#include <pthread.h>
#include <stdio.h>
#include <set>
#include <unordered_map>
#include <vector>
#include "pdf_struct.h"
#ifdef __cplusplus
extern "C"
{
#endif
    namespace pdf
    {
    class ParserSingleton
    {
    public:
        static ParserSingleton *GetInstance();

    public:
        void Destroy();
        //xref
        std::vector<pdf::XrefTable> GetXref() const;
        void SetXref(const std::vector<pdf::XrefTable> &input);
        //obj
        std::unordered_map<std::string, std::string> GetDictionary() const;
        void SetDictionary(std::unordered_map<std::string, std::string> input);

        int GetObjName() const;
        void SetObjName(int name);
        //parent data
        std::vector<int> GetKidsObj() const;
        void SetKidsObj(const std::vector<int> &input);

        int GetPageSum() const;
        void SetPageSum(int input);

        int GetResouresObj() const;
        void SetResouresObj(int input);
        //FILE
        FILE *GetFilePointer() const;
        void SetFilePointer(FILE *input);

        //stream
        char *Getstream();
        void Setstream(char *input, int length);

        //outfile path
        void SetOutFilePath(std::string input);
        std::string GetOutFilePath() const;

    private:
        //xref
        std::vector<pdf::XrefTable> xref_vec;
        //kids object number
        std::vector<int> kid_obj_vec_;
        //parser dict data
        std::unordered_map<std::string, std::string> dict_map;
        //file
        FILE *file_ptr_;
        std::string out_file_path_;
        char *parser_stream_ptr_;
        //obj
        int object_number;
        //parent data
        int resoures_obj_num_;
        int page_sum_;

    private:
        ParserSingleton();
        virtual ~ParserSingleton();
        ParserSingleton &operator=(const ParserSingleton &singleton){};

    private:
        static ParserSingleton *m_pInstance;
        static pthread_mutex_t mutex;
    };
    } //namespace pdf
#ifdef __cplusplus
}
#endif
#endif // _PDF_BASICDATA_SINGLETON_H