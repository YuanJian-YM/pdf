#include "pdf_parser.h"
namespace pdf
{
bool PdfParser::IndirectObjectParser(FILE *file_ptr, int &output_object)
{
    char temp;
    std::string contents;
    while (temp = fgetc(file_ptr))
    {
        if (temp == '\n' || temp == '\r')
            break;
        else
            contents.push_back(temp);
    }
    if (contents.find("obj") == contents.npos)
        return false;
    std::stringstream sstream_obj(contents);
    std::string obj_string;
    sstream_obj >> obj_string;
    output_object = atoi(obj_string.c_str());
    return true;
}
bool PdfParser::DictionaryParser(const std::string &input, std::unordered_map<std::string, std::string> &output_map)
{
    /*     bool is_key_bool = true;
    is_end_dict_parser_ = false;
    std::string key_string, value_string;
    for (int i = 0; i < input.size(); ++i) {
        char temp = input[i];
        if (temp == '\n' || temp == '\r')
            continue;
        if (temp == '>')
            break;
        if (is_key_bool) {
            key_string.clear();
            if (temp == '/') {
                while (true) {
                    temp = fgetc(file_ptr);
                    if (temp == '/' || temp == ' ' || temp == '(' || temp == '[' || temp == '<') {
                        fseek(file_ptr, -1L, SEEK_CUR);
                        break;
                    }
                    key_string += temp;
                }
            }
            is_key_bool = false;
        }
    } */
}
bool PdfParser::DictionaryParser(FILE *file_ptr, std::unordered_map<std::string, std::string> &output_map)
{
    bool is_key_bool = true;
    is_end_dict_parser_ = false;
    std::string key_string, value_string;
    while (true)
    {
        char temp = fgetc(file_ptr);
        if (temp == '\n' || temp == '\r')
            continue;
        if (temp == '>')
            break;
        if (is_key_bool)
        {
            key_string.clear();
            if (temp == '/')
            {
                while (true)
                {
                    temp = fgetc(file_ptr);
                    if (temp == '/' || temp == ' ' || temp == '(' || temp == '[' || temp == '<')
                    {
                        fseek(file_ptr, -1L, SEEK_CUR);
                        break;
                    }
                    key_string += temp;
                }
            }
            is_key_bool = false;
        }
        else
        {
            if (temp == ' ')
                continue;
            value_string.clear();
            switch (temp)
            {
            case '/':
                value_string = GetStringByFile(file_ptr);
                break;
            case '(':
                fseek(file_ptr, -1L, SEEK_CUR);
                value_string = GetStringByFile(file_ptr, '(', ')');
                break;
            case '[':
                fseek(file_ptr, -1L, SEEK_CUR);
                value_string = GetStringByFile(file_ptr, '[', ']');
                break;
            case '<':
                fseek(file_ptr, 1L, SEEK_CUR);
                value_string = DictDict(file_ptr);
                break;
            default:
                fseek(file_ptr, -1L, SEEK_CUR);
                value_string = GetStringByFile(file_ptr);
                break;
            }
            output_map.insert(std::make_pair(key_string, value_string));
            if (is_end_dict_parser_)
                break;
            is_key_bool = true;
        }
    }
    return true;
}
bool PdfParser::StreamParser(FILE *file_ptr, std::string &length_string)
{
    int file_offset = ftell(file_ptr);
    int length;
    if (length_string.find('R') != length_string.npos)
    {
        int length_obj = pdf::GetObjectNumber(length_string);
        fseek(file_ptr, pdf::ParserSingleton::GetInstance()->GetXref()[length_obj].object_offset_int, SEEK_SET);
        if (GetLengthOfLengthObject(file_ptr, length) == false)
        {
            std::cout << "获取流长度失败" << std::endl;
            return false;
        }
    }
    else
    {
        length = atoi(length_string.c_str());
    }
    length_string = std::to_string(length);
    fseek(file_ptr, file_offset, SEEK_SET);
    {
        char *content = new char[length];
        fseek(file_ptr, file_offset, SEEK_SET);
        fread(content, 1, length, file_ptr);
        pdf::ParserSingleton::GetInstance()->Setstream(content, length);
        delete[] content;
        content = nullptr;
    }
    return true;
}
std::string PdfParser::GetStringByFile(FILE *file_ptr)
{
    std::string value_string;
    char temp;
    while (true)
    {
        temp = fgetc(file_ptr);
        if (temp == '\n' || temp == '\r')
            continue;
        if (temp == '/' || temp == '>')
        {
            fseek(file_ptr, -1L, SEEK_CUR);
            if (temp == '>')
            {
                is_end_dict_parser_ = true;
                fseek(file_ptr, 2L, SEEK_CUR);
            }
            return value_string;
        }
        value_string += temp;
    }
};
std::string PdfParser::GetStringByFile(FILE *file_ptr, char input, char end_input)
{
    int count = 0;
    std::string value_string;
    char temp;
    while (true)
    {
        temp = fgetc(file_ptr);
        if (temp == '\n' || temp == '\r')
            continue;
        if (temp == input)
            ++count;
        if (temp == end_input && (--count == 0))
        {
            return value_string += temp;
        }
        value_string += temp;
    }
}
std::string PdfParser::DictDict(FILE *file_ptr)
{
    std::string value_string;
    int count = 1;
    value_string += "<<";
    char temp;
    while (true)
    {
        temp = fgetc(file_ptr);
        if (temp == '\n' || temp == '\r')
            continue;
        if (temp == '<')
        {
            char temp_array[2];
            temp_array[0] = temp;
            temp_array[1] = fgetc(file_ptr);
            if (strstr(temp_array, "<<") != NULL)
            {
                ++count;
                value_string += "<<";
            }
            else
            {
                value_string.append(temp_array);
            }
        }
        else if (temp == '>')
        {
            char temp_array[2];
            temp_array[0] = temp;
            temp_array[1] = fgetc(file_ptr);
            if (strstr(temp_array, ">>") != NULL)
            {
                --count;
                value_string += ">>";
                if (count == 0)
                {
                    while (true)
                    {
                        temp = fgetc(file_ptr);
                        if (temp == '\n' || temp == '\r')
                            continue;
                        else
                            break;
                    }
                    if (temp == '>')
                    {
                        fseek(file_ptr, 1L, SEEK_CUR);
                        is_end_dict_parser_ = true;
                    }
                    else
                    {
                        fseek(file_ptr, -1L, SEEK_CUR);
                    }
                    return value_string;
                }
            }
            else
            {
                value_string.append(temp_array);
            }
        }
        else
        {
            value_string += temp;
        }
    }
};

bool PdfParser::ObjectParser(FILE *file_ptr)
{
    int object_number;
    if (!IndirectObjectParser(file_ptr, object_number))
        return false;
    pdf::ParserSingleton::GetInstance()->SetObjName(object_number);
    char temp;
    for (int i = 0; i < 4; ++i)
    {
        temp = fgetc(file_ptr);
        if (temp == '\n' || temp == '\r')
            continue;
        if (temp == '<' && fgetc(file_ptr) == '<')
        {
            std::unordered_map<std::string, std::string> dict_map;
            DictionaryParser(file_ptr, dict_map);
            pdf::ParserSingleton::GetInstance()->SetDictionary(dict_map);
            continue;
        }
        if (temp == 's')
        {
            fseek(file_ptr, -1L, SEEK_CUR);
            char stream_begin_array[kLineSizeObject];
            fgets(stream_begin_array, kLineSizeObject, file_ptr);
            if (strstr(stream_begin_array, "stream") != NULL)
            {
                auto dict_map = pdf::ParserSingleton::GetInstance()->GetDictionary();
                std::unordered_map<std::string, std::string>::iterator iter;
                if ((iter = dict_map.find("Length")) != dict_map.end())
                {
                    if (StreamParser(file_ptr, iter->second))
                        pdf::ParserSingleton::GetInstance()->SetDictionary(dict_map);
                }
            }
        }
    }
    return true;
};
bool PdfParser::GetLengthOfLengthObject(FILE *file_ptr, int &length)
{
    int object_number;
    if (!IndirectObjectParser(file_ptr, object_number))
        return false;
    char line_content_array[kLineSizeLength];
    if (fgets(line_content_array, kLineSizeLength, file_ptr) == NULL)
        return false;
    length = atoi(line_content_array);
    return true;
};
bool PdfParser::GetXrefTable(FILE *file_ptr, std::vector<pdf::XrefTable> &output_vector)
{
    char line_content_array[kLineSizeXref];
    if (fgets(line_content_array, kLineSizeXref, file_ptr) == NULL)
        return false;
    if (strstr(line_content_array, "xref") == NULL)
        return false;
    memset(line_content_array, 0, kLineSizeXref);
    if (fgets(line_content_array, kLineSizeXref, file_ptr) == NULL)
        return false;
    int start_object_number, object_sum;
    std::stringstream sstream_obj(line_content_array);
    std::string obj_string;
    for (int i = 0; i < 2; ++i)
    {
        sstream_obj >> obj_string;
        if (i == 0)
            start_object_number = atoi(obj_string.c_str());
        else
            object_sum = atoi(obj_string.c_str());
    }
    output_vector.reserve(object_sum);
    for (int i = 0; i < object_sum; ++i)
    {
        pdf::XrefTable xreftable;
        xreftable.object_number_int = start_object_number++;
        memset(line_content_array, 0, kLineSizeXref);
        if (fgets(line_content_array, kLineSizeXref, file_ptr) == NULL)
            continue;
        std::string temp_string;
        std::stringstream temp_sstream(line_content_array);
        for (int j = 0; j < 3; ++j)
        {
            temp_sstream >> temp_string;
            if (j == 0)
                xreftable.object_offset_int = atoi(temp_string.c_str());
            else if (j == 1)
                xreftable.object_use_times_int = atoi(temp_string.c_str());
            else
                xreftable.is_useful_char = temp_string;
        }
        output_vector.push_back(xreftable);
    }
    return true;
};
std::string PdfParser::ReadLineData(FILE *file_ptr)
{
    std::string data;
    while (true)
    {
        char temp = fgetc(file_ptr);
        fseek(file_ptr, -2L, SEEK_CUR);
        if (temp == '\n' || temp == '\r')
        {
            reverse(data.begin(), data.end());
            return data;
        }
        else
        {
            data += temp;
        }
    }
};
int PdfParser::GetStartXref(FILE *file_ptr)
{
    fseek(file_ptr, -1L, SEEK_END);
    std::string data;
    int file_offset;
    while (true)
    {
        data = ReadLineData(file_ptr);
        if (data.find("startxref") != data.npos)
        {
            char xref_offset[kLineSizeObject];
            fseek(file_ptr, file_offset + 2, SEEK_SET);
            if (fgets(xref_offset, kLineSizeObject, file_ptr) != NULL)
            {
                return atoi(xref_offset);
            }
            else
            {
                return -1;
            }
        }
        file_offset = ftell(file_ptr);
    }
};
bool PdfParser::GetTrailer(FILE *file_ptr)
{
    fseek(file_ptr, -1L, SEEK_END);
    std::string data;
    while (true)
    {
        data = ReadLineData(file_ptr);
        if (data.find("trailer") != data.npos)
        {
            while (true)
            {
                char temp = fgetc(file_ptr);
                if (temp == '<')
                {
                    if (temp == '<' && fgetc(file_ptr) == '<')
                    {
                        std::unordered_map<std::string, std::string> dict_map;
                        DictionaryParser(file_ptr, dict_map);
                        pdf::ParserSingleton::GetInstance()->SetDictionary(dict_map);
                        break;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
            break;
        }
    }
    return true;
};

} //namespace pdf