#include "pdf_chile_object_analysis.h"
namespace pdf
{
PdfPageAnalysis::PdfPageAnalysis(int number) : contents_obj_number_(0)
{
    pdf::PdfParser parser;
    if (number > pdf::ParserSingleton::GetInstance()->GetPageSum())
    {
        std::cout << "输入的页数超过总数" << std::endl;
        exit(-1);
    }
    int obj_number = pdf::ParserSingleton::GetInstance()->GetKidsObj()[number - 1];
    file_ptr_ = pdf::ParserSingleton::GetInstance()->GetFilePointer();
    fseek(file_ptr_, pdf::ParserSingleton::GetInstance()->GetXref()[obj_number].object_offset_int, SEEK_SET);
    parser.ObjectParser(file_ptr_);
    AnalysisChild();
}
PdfPageAnalysis::~PdfPageAnalysis()
{
}
std::unordered_map<std::string, int> PdfPageAnalysis::ParserXobject(const std::string &input)
{
    std::unordered_map<std::string, int> xobject_map;
    std::string temp_str;
    for (auto iter = input.begin(); iter != input.end(); ++iter)
    {
        if (*iter == '\n' || *iter == '<')
            continue;
        if (*iter == '/' || *iter == '>')
        {
            if (temp_str.empty())
                continue;
            std::stringstream stream(temp_str);
            std::vector<std::string> result_vec;
            std::string result;
            for (int i = 0; i < 2; ++i)
            {
                stream >> result;
                result_vec.push_back(result);
            }
            xobject_map.insert(std::make_pair(result_vec[0], atoi(result_vec[1].c_str())));
            temp_str.clear();
        }
        else
        {
            temp_str += *iter;
        }
    }
    return xobject_map;
}
void PdfPageAnalysis::AnalysisChild()
{
    pdf::PdfParser parser;
    auto child_map = pdf::ParserSingleton::GetInstance()->GetDictionary();
    //Resources
    std::unordered_map<std::string, std::string>::iterator dict_iter;
    if ((dict_iter = child_map.find("Resources")) != child_map.end())
    {
        if (dict_iter->second.size() < 12)
        {
            int resources_obj = pdf::GetObjectNumber(dict_iter->second);
            fseek(file_ptr_, pdf::ParserSingleton::GetInstance()->GetXref()[resources_obj].object_offset_int, SEEK_SET);
            parser.ObjectParser(file_ptr_);
            auto resources_map = pdf::ParserSingleton::GetInstance()->GetDictionary();
            if ((dict_iter = resources_map.find("XObject")) != resources_map.end())
            {
                xobject_map_ = ParserXobject(dict_iter->second);
            }
            //font
            if ((dict_iter = resources_map.find("Font")) != resources_map.end())
            {
                int font_obj = pdf::GetObjectNumber(dict_iter->second);
                fseek(file_ptr_, pdf::ParserSingleton::GetInstance()->GetXref()[font_obj].object_offset_int, SEEK_SET);
                parser.ObjectParser(file_ptr_);
                auto font_map = pdf::ParserSingleton::GetInstance()->GetDictionary();
                for (auto iter = font_map.begin(); iter != font_map.end(); ++iter)
                {
                    font_object_.insert(std::make_pair(iter->first, pdf::GetObjectNumber(iter->second)));
                }
            }
        }
        else
        {
        }
    }
    //MediaBox
    if ((dict_iter = child_map.find("MediaBox")) != child_map.end())
    {
        std::string box_str;
        for (auto media_iter = dict_iter->second.begin(); media_iter != dict_iter->second.end(); ++media_iter)
        {
            if (*media_iter == '[' || *media_iter == ']')
                continue;
            else
                box_str += *media_iter;
        }
        std::stringstream stream(box_str);
        while (stream >> box_str)
        {
            mediabox_.push_back(atoi(box_str.c_str()));
        }
    }
    //Device
    if ((dict_iter = child_map.find("Group")) != child_map.end())
    {
        std::string box_str;
        for (auto device_iter = dict_iter->second.begin(); device_iter != dict_iter->second.end(); ++device_iter)
        {
            if (*device_iter == '<' || *device_iter == '>')
                continue;
            if (*device_iter == '/')
            {
                if (box_str.find("Device") != box_str.npos)
                {
                    colorspace_ = box_str;
                    break;
                }
                else
                {
                    box_str.clear();
                }
            }
            else
            {
                box_str += *device_iter;
            }
        }
    }
    //Contents
    if ((dict_iter = child_map.find("Contents")) != child_map.end())
    {
        contents_obj_number_ = pdf::GetObjectNumber(dict_iter->second);
    }
}
std::unordered_map<std::string, int> PdfPageAnalysis::GetFontObject() const
{
    return font_object_;
}
std::unordered_map<std::string, int> PdfPageAnalysis::GetXObjectObject() const
{
    return xobject_map_;
}
std::vector<double> PdfPageAnalysis::GetMediaBox() const
{
    return mediabox_;
}
std::string PdfPageAnalysis::GetDevice() const
{
    return colorspace_;
}
int PdfPageAnalysis::GetContentsObject() const
{
    return contents_obj_number_;
}
} //namespace pdf