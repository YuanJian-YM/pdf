#include "pdf_basicdata_singleton.h"
namespace pdf
{
pthread_mutex_t ParserSingleton::mutex;
ParserSingleton *ParserSingleton::m_pInstance = nullptr;
ParserSingleton::ParserSingleton() : parser_stream_ptr_(nullptr)
{
}
ParserSingleton::~ParserSingleton()
{
}
ParserSingleton *ParserSingleton::GetInstance()
{
    if (m_pInstance == NULL)
    {
        pthread_mutex_init(&mutex, NULL);
        pthread_mutex_lock(&mutex);
        if (m_pInstance == NULL)
            m_pInstance = new ParserSingleton();
        pthread_mutex_unlock(&mutex);
    }
    return m_pInstance;
}
void ParserSingleton::Destroy()
{
    if (parser_stream_ptr_ != nullptr)
    {
        delete[] parser_stream_ptr_;
        parser_stream_ptr_ = nullptr;
    }
    pthread_mutex_destroy(&mutex);
    if (m_pInstance != nullptr)
    {
        delete m_pInstance;
        m_pInstance = nullptr;
    }
}
std::vector<pdf::XrefTable> ParserSingleton::GetXref() const
{
    return xref_vec;
}
void ParserSingleton::SetXref(const std::vector<pdf::XrefTable> &input)
{
    xref_vec = input;
}
std::unordered_map<std::string, std::string> ParserSingleton::GetDictionary() const
{
    return dict_map;
}
void ParserSingleton::SetDictionary(std::unordered_map<std::string, std::string> input)
{
    dict_map = input;
}
int ParserSingleton::GetObjName() const
{
    return object_number;
}
void ParserSingleton::SetObjName(int name)
{
    object_number = name;
}
std::vector<int> ParserSingleton::GetKidsObj() const
{
    return kid_obj_vec_;
}
void ParserSingleton::SetKidsObj(const std::vector<int> &input)
{
    kid_obj_vec_ = input;
}

int ParserSingleton::GetPageSum() const
{
    return page_sum_;
}
void ParserSingleton::SetPageSum(int input)
{
    page_sum_ = input;
}

int ParserSingleton::GetResouresObj() const
{
    return resoures_obj_num_;
}
void ParserSingleton::SetResouresObj(int input)
{
    resoures_obj_num_ = input;
}
FILE *ParserSingleton::GetFilePointer() const
{
    return file_ptr_;
}
void ParserSingleton::SetFilePointer(FILE *input)
{
    file_ptr_ = input;
}
char *ParserSingleton::Getstream()
{
    return parser_stream_ptr_;
}
void ParserSingleton::Setstream(char *input, int length)
{
    if (parser_stream_ptr_ != nullptr)
    {
        delete[] parser_stream_ptr_;
        parser_stream_ptr_ = nullptr;
    }
    parser_stream_ptr_ = new char[length];
    memcpy(parser_stream_ptr_, input, length);
}
void ParserSingleton::SetOutFilePath(std::string input)
{
    out_file_path_ = input;
}
std::string ParserSingleton::GetOutFilePath() const
{
    return out_file_path_;
}
} // namespace pdf