#include "pdf_base_init.h"
namespace pdf
{
PdfBaseInit::PdfBaseInit(std::string input_file, std::string out_file)
{
    file_ptr_ = nullptr;
    if ((file_ptr_ = fopen(input_file.c_str(), "r")) == nullptr)
    {
        std::cout << "打开文件失败" << std::endl;
        exit(-1);
    }
    if (access(out_file.c_str(), 00) == -1)
    {
        std::cout << "输出路径有误" << std::endl;
        exit(-1);
    }
    pdf::ParserSingleton::GetInstance()->SetFilePointer(file_ptr_);
    pdf::ParserSingleton::GetInstance()->SetOutFilePath(out_file);
}
void PdfBaseInit::GetXrefTable(pdf::PdfParser parser)
{
    fseek(file_ptr_, xref_offset_, SEEK_SET);
    std::vector<pdf::XrefTable> xref_table;
    if (parser.GetXrefTable(file_ptr_, xref_table) == false)
    {
        std::cout << "无法获取交叉索引表" << std::endl;
    }
    else
    {
        pdf::ParserSingleton::GetInstance()->SetXref(xref_table);
    }
}
void PdfBaseInit::GetTrailerAndStartXref(pdf::PdfParser parser)
{
    int start_xref = parser.GetStartXref(file_ptr_);
    if (start_xref == -1)
        std::cout << "startxref 获取失败" << std::endl;
    if (!parser.GetTrailer(file_ptr_))
    {
        std::cout << "trailer 获取失败" << std::endl;
        exit(-1);
    }
    auto trailer_map = pdf::ParserSingleton::GetInstance()->GetDictionary();
    std::unordered_map<std::string, std::string>::iterator iter;
    if ((iter = trailer_map.find("XRefStm")) != trailer_map.end())
    {
        xref_offset_ = atoi(iter->second.c_str());
    }
    else
    {
        if (start_xref <= 0)
        {
            std::cout << "无法获取交叉引用表" << std::endl;
            exit(-1);
        }
        xref_offset_ = start_xref;
    }
}
void PdfBaseInit::GetRootObj()
{
    auto trailer_map = pdf::ParserSingleton::GetInstance()->GetDictionary();
    auto iter = trailer_map.find("Root");
    if (iter != trailer_map.end())
    {
        if (iter->second.find('R') != iter->second.npos)
        {
            root_number = pdf::GetObjectNumber(iter->second);
        }
        else
        {
            root_number = atoi(iter->second.c_str());
            if (root_number <= 0)
            {
                std::cout << "无法获取根节点" << std::endl;
                exit(-1);
            }
        }
    }
}
void PdfBaseInit::GetKids(pdf::PdfParser parser)
{
    fseek(file_ptr_, pdf::ParserSingleton::GetInstance()->GetXref()[root_number].object_offset_int, SEEK_SET);
    if (!parser.ObjectParser(file_ptr_))
    {
        std::cout << "无法解析根对象" << std::endl;
        exit(-1);
    }
    auto root_map = pdf::ParserSingleton::GetInstance()->GetDictionary();
    auto type = root_map.find("type");
    if (type != root_map.end())
    {
        if (type->second != "Catalog")
        {
            std::cout << "无法解析根对象" << std::endl;
            exit(-1);
        }
    }
    auto page = root_map.find("Pages");
    int parent_obj_num;
    if (page != root_map.end())
    {
        if (page->second.find('R'))
        {
            parent_obj_num = pdf::GetObjectNumber(page->second);
        }
        else
        {
            parent_obj_num = atoi(page->second.c_str());
        }
    }
    fseek(file_ptr_, pdf::ParserSingleton::GetInstance()->GetXref()[parent_obj_num].object_offset_int, SEEK_SET);
    if (!parser.ObjectParser(file_ptr_))
    {
        std::cout << "无法解析父页对象" << std::endl;
        exit(-1);
    }
    auto parent_map = pdf::ParserSingleton::GetInstance()->GetDictionary();
    auto resoures_iter = parent_map.find("Resources");
    if (resoures_iter != parent_map.end())
    {
        if (resoures_iter->second.find('R'))
        {
            pdf::ParserSingleton::GetInstance()->SetResouresObj(pdf::GetObjectNumber(resoures_iter->second));
        }
        else
        {
            pdf::ParserSingleton::GetInstance()->SetResouresObj(atoi(resoures_iter->second.c_str()));
        }
    }
    auto count_iter = parent_map.find("Count");
    if (count_iter != parent_map.end())
        pdf::ParserSingleton::GetInstance()->SetPageSum(atoi(count_iter->second.c_str()));
    auto kid_iter = parent_map.find("Kids");
    if (kid_iter != parent_map.end())
    {
        pdf::ParserSingleton::GetInstance()->SetKidsObj(AnalysisKids(kid_iter->second));
    }
    else
    {
        std::cout << "无法获取子页" << std::endl;
        exit(-1);
    }
}
void PdfBaseInit::PdfClose()
{
    fclose(file_ptr_);
}
void PdfBaseInit::Init()
{
    pdf::PdfParser parser;
    GetTrailerAndStartXref(parser);
    GetXrefTable(parser);
    GetRootObj();
    GetKids(parser);
}
std::vector<int> PdfBaseInit::AnalysisKids(const std::string &input)
{
    std::vector<int> obj_vec;
    std::string obj_str;
    for (auto iter = input.begin(); iter != input.end(); ++iter)
    {
        if (*iter == '[' || *iter == ']')
            continue;
        if (*iter == 'R')
        {
            obj_str += *iter;
            obj_vec.push_back(pdf::GetObjectNumber(obj_str));
            obj_str.clear();
        }
        else
        {
            obj_str += *iter;
        }
    }
    return obj_vec;
}
} // namespace pdf