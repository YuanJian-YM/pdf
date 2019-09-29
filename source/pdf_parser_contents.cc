#include "pdf_parser_contents.h"
namespace pdf
{
PdfParserContents::PdfParserContents(int pagenumber) : word_ptr_(nullptr), line_ptr_(nullptr)
{
    data_find_map_["RG"] = std::bind(&PdfParserContents::DoRG, this, std::placeholders::_1);
    data_find_map_["w"] = std::bind(&PdfParserContents::Dow, this, std::placeholders::_1);
    data_find_map_["m"] = std::bind(&PdfParserContents::Dom, this, std::placeholders::_1);
    data_find_map_["l"] = std::bind(&PdfParserContents::Dol, this, std::placeholders::_1);
    data_find_map_["S"] = std::bind(&PdfParserContents::DoS, this, std::placeholders::_1);
    data_find_map_["j"] = std::bind(&PdfParserContents::DoOther, this, std::placeholders::_1);
    data_find_map_["J"] = std::bind(&PdfParserContents::DoOther, this, std::placeholders::_1);
    data_find_map_["q"] = std::bind(&PdfParserContents::DoOther, this, std::placeholders::_1);
    data_find_map_["W*"] = std::bind(&PdfParserContents::DoOther, this, std::placeholders::_1);
    data_find_map_["n"] = std::bind(&PdfParserContents::DoOther, this, std::placeholders::_1);
    data_find_map_["rg"] = std::bind(&PdfParserContents::DoRg, this, std::placeholders::_1);
    data_find_map_["l"] = std::bind(&PdfParserContents::Dol, this, std::placeholders::_1);
    data_find_map_["h"] = std::bind(&PdfParserContents::DoOther, this, std::placeholders::_1);
    data_find_map_["f*"] = std::bind(&PdfParserContents::DoOther, this, std::placeholders::_1);
    data_find_map_["S"] = std::bind(&PdfParserContents::DoS, this, std::placeholders::_1);
    data_find_map_["re"] = std::bind(&PdfParserContents::DoRe, this, std::placeholders::_1);
    data_find_map_["Do"] = std::bind(&PdfParserContents::DoDo, this, std::placeholders::_1);
    data_find_map_["cm"] = std::bind(&PdfParserContents::DoCm, this, std::placeholders::_1);
    data_find_map_["BT"] = std::bind(&PdfParserContents::DoBT, this, std::placeholders::_1);
    data_find_map_["Tm"] = std::bind(&PdfParserContents::DoTm, this, std::placeholders::_1);
    data_find_map_["Tf"] = std::bind(&PdfParserContents::DoTf, this, std::placeholders::_1);
    data_find_map_["Td"] = std::bind(&PdfParserContents::DoTd, this, std::placeholders::_1);
    data_find_map_["TD"] = std::bind(&PdfParserContents::DoTD, this, std::placeholders::_1);
    data_find_map_["Tj"] = std::bind(&PdfParserContents::DoTj, this, std::placeholders::_1);
    data_find_map_["TJ"] = std::bind(&PdfParserContents::DoTJ, this, std::placeholders::_1);
    data_find_map_["ET"] = std::bind(&PdfParserContents::DoET, this, std::placeholders::_1);
    file_ptr_ = pdf::ParserSingleton::GetInstance()->GetFilePointer();
    child_page_ptr_ = std::make_shared<PdfPageAnalysis>(pagenumber);
    image_ptr_ = std::make_shared<pdf::PdfImage>();
    text_ptr_ = std::make_shared<pdf::PdfText>();
    //get font data
    DealFontData();
    //content
    DealContentsData();
}
PdfParserContents::~PdfParserContents(){};
void PdfParserContents::DoRe(std::string input){};
void PdfParserContents::DoCm(std::string input)
{
    positon_vec_.clear();
    std::stringstream stream(input);
    std::string temp_str;
    while (stream >> temp_str)
        positon_vec_.push_back(atof(temp_str.c_str()));
};
void PdfParserContents::DoRG(std::string input)
{
    if (line_ptr_ != nullptr)
        delete line_ptr_;
    line_ptr_ = new pdf::LineData;
    std::stringstream stream(input);
    std::string temp_str;
    while (stream >> temp_str)
        line_ptr_->color.push_back(atof(temp_str.c_str()));
}
void PdfParserContents::Dow(std::string input)
{
    if (line_ptr_ != nullptr)
        line_ptr_->thick = atof(input.c_str());
}
void PdfParserContents::Dom(std::string input)
{
    if (line_ptr_ == nullptr)
        return;
    std::stringstream stream(input);
    std::string temp_str;
    pdf::Point2D point;
    int index = 1;
    while (stream >> temp_str)
    {
        if (index % 2 != 0)
        {
            point.x = round(atof(temp_str.c_str()));
        }
        else
        {
            point.y = round(atof(temp_str.c_str()));
            line_ptr_->point_m = point;
        }
        ++index;
    }
}
void PdfParserContents::Dol(std::string input)
{
    if (line_ptr_ == nullptr)
        return;
    std::stringstream stream(input);
    std::string temp_str;
    pdf::Point2D point;
    int index = 1;
    while (stream >> temp_str)
    {
        if (index % 2 != 0)
        {
            point.x = round(atof(temp_str.c_str()));
        }
        else
        {
            point.y = round(atof(temp_str.c_str()));
            line_ptr_->point_l.push_back(point);
        }
        ++index;
    }
}
void PdfParserContents::DoS(std::string input)
{
    if (line_ptr_ != nullptr)
    {
        text_ptr_->SetLinedata(*line_ptr_);
        delete line_ptr_;
        line_ptr_ = nullptr;
    }
}
void PdfParserContents::DoDo(std::string input)
{
    std::string input_str = "Im";
    input_str.append(input);
    std::stringstream stream(input_str);
    stream >> input_str;
    pdf::PdfParser parser;
    auto xobject_map = child_page_ptr_->GetXObjectObject();
    std::unordered_map<std::string, int>::iterator iter;
    int image_number;
    if ((iter = xobject_map.find(input_str)) != xobject_map.end())
    {
        image_number = iter->second;
    }
    else
    {
        std::cout << "没找到图片对象" << std::endl;
        return;
    }
    fseek(file_ptr_, pdf::ParserSingleton::GetInstance()->GetXref()[image_number].object_offset_int, SEEK_SET);
    parser.ObjectParser(file_ptr_);
    ImageData image_data;
    auto image_map = pdf::ParserSingleton::GetInstance()->GetDictionary();
    FillImage(image_map, image_data);
    unsigned char *dest_ptr = new unsigned char[kUncompressSize];
    size_t dest_length = kUncompressSize;
    pdf::PdfUncompress pdf_uncompress;
    if (pdf_uncompress.PdfUncompressData((unsigned char *)pdf::ParserSingleton::GetInstance()->Getstream(), image_data.length, dest_ptr, dest_length, image_data.filter))
    {
        image_data.streamlength = dest_length;
        image_data.iamge_str = (char *)dest_ptr;
    }
    else
    {
        return;
    }

    //smask
    int smask_obj;
    if (image_data.sMask.find('R') != image_data.sMask.npos)
    {
        smask_obj = pdf::GetObjectNumber(image_data.sMask);
    }
    else
    {
        smask_obj = atoi(image_data.sMask.c_str());
    }
    fseek(file_ptr_, pdf::ParserSingleton::GetInstance()->GetXref()[smask_obj].object_offset_int, SEEK_SET);
    parser.ObjectParser(file_ptr_);
    auto smask_map = pdf::ParserSingleton::GetInstance()->GetDictionary();
    FillImageSmask(smask_map, image_data.smask_data);
    unsigned char *dest_smask_ptr = new unsigned char[kUncompressSize];
    size_t dest_smask_length = kUncompressSize;
    if (pdf_uncompress.PdfUncompressData((unsigned char *)pdf::ParserSingleton::GetInstance()->Getstream(), image_data.smask_data.length, dest_smask_ptr, dest_smask_length, image_data.smask_data.filter))
    {
        image_data.smask_data.streamlength = dest_smask_length;
        image_data.smask_data.iamge_str = (char *)dest_smask_ptr;
    }
    auto image_data_map = image_ptr_->GetImageData();
    image_data.cm_vec = positon_vec_;
    image_data_map.push_back(image_data);
    image_ptr_->SetImageData(image_data_map);

    delete[] dest_ptr;
    dest_ptr = nullptr;
    delete[] dest_smask_ptr;
    dest_smask_ptr = nullptr;
    image_data.iamge_str = nullptr;
    image_data.smask_data.iamge_str = nullptr;
};
void PdfParserContents::DoRg(std::string input)
{
    color_vec_.clear();
    std::stringstream stream(input);
    std::string contents;
    while (stream >> contents)
        color_vec_.push_back(255 * atof(contents.c_str()));
};

void PdfParserContents::DoBT(std::string input)
{
    if (word_ptr_ != nullptr)
    {
        delete word_ptr_;
        word_ptr_ = nullptr;
    }
    word_ptr_ = new pdf::WordData;
    if (!color_vec_.empty())
        word_ptr_->color = color_vec_;
};
void PdfParserContents::DoTm(std::string input)
{
    if (word_ptr_ == nullptr)
        return;
    std::stringstream stream(input);
    std::string contents;
    while (stream >> contents)
        word_ptr_->tm.push_back(atof(contents.c_str()));
};
void PdfParserContents::DoTd(std::string input)
{
    if (word_ptr_ == nullptr)
        return;
    std::stringstream stream(input);
    std::string contents;
    while (stream >> contents)
        word_ptr_->td.push_back(atof(contents.c_str()));
};
void PdfParserContents::DoTD(std::string input)
{
    DoTd(input);
};
void PdfParserContents::DoTf(std::string input)
{
    if (word_ptr_ == nullptr)
        return;
    input.erase(std::remove(input.begin(), input.end(), '/'));
    std::stringstream stream(input);
    std::string contents;
    std::vector<std::string> temp_vec;
    while (stream >> contents)
        temp_vec.push_back(contents);
    if (temp_vec.size() < 2)
        return;
    word_ptr_->tf.insert(std::make_pair(temp_vec[0], atoi(temp_vec[1].c_str())));
    word_ptr_->tf_order.push_back(temp_vec[0]);
}
void PdfParserContents::DoTj(std::string input)
{
    if (word_ptr_ == nullptr)
        return;
    auto data = ParserTj(input);
    word_ptr_->tj.push_back(data);
};
void PdfParserContents::DoTJ(std::string input)
{
    DoTj(input);
};
void PdfParserContents::DoET(std::string input)
{
    auto data = text_ptr_->GetTextData();
    data.push_back(*word_ptr_);
    text_ptr_->SetTextData(data);
    if (word_ptr_ != nullptr)
    {
        delete word_ptr_;
        word_ptr_ = nullptr;
    }
};
void PdfParserContents::FillImage(const std::unordered_map<std::string, std::string> &input, ImageData &image_data)
{
    for (auto image_iter = input.begin(); image_iter != input.end(); ++image_iter)
    {
        if (image_iter->first == "Type")
            image_data.type = image_iter->second;
        else if (image_iter->first == "Subtype")
            image_data.subtype = image_iter->second;
        else if (image_iter->first == "Width")
            image_data.width = atoi(image_iter->second.c_str());
        else if (image_iter->first == "Height")
            image_data.height = atoi(image_iter->second.c_str());
        else if (image_iter->first == "BitsPerComponent")
            image_data.bitsPerComponent = atoi(image_iter->second.c_str());
        else if (image_iter->first == "Filter")
            image_data.filter = image_iter->second;
        else if (image_iter->first == "ColorSpace")
            image_data.colorspace = image_iter->second;
        else if (image_iter->first == "SMask")
            image_data.sMask = image_iter->second;
        else if (image_iter->first == "Length")
            image_data.length = atoi(image_iter->second.c_str());
    }
};
void PdfParserContents::FillImageSmask(const std::unordered_map<std::string, std::string> &input, ImageData::SMask &image_data)
{
    for (auto image_iter = input.begin(); image_iter != input.end(); ++image_iter)
    {
        if (image_iter->first == "Type")
            image_data.type = image_iter->second;
        else if (image_iter->first == "Subtype")
            image_data.subtype = image_iter->second;
        else if (image_iter->first == "Width")
            image_data.width = atoi(image_iter->second.c_str());
        else if (image_iter->first == "Height")
            image_data.height = atoi(image_iter->second.c_str());
        else if (image_iter->first == "BitsPerComponent")
            image_data.bitsPerComponent = atoi(image_iter->second.c_str());
        else if (image_iter->first == "Filter")
            image_data.filter = image_iter->second;
        else if (image_iter->first == "ColorSpace")
            image_data.colorspace = image_iter->second;
        else if (image_iter->first == "Length")
            image_data.length = atoi(image_iter->second.c_str());
        else if (image_iter->first == "Decode")
            image_data.decode = image_iter->second;
    }
}
void PdfParserContents::ParserContents(char *input, int length)
{
    std::cout << input << std::endl;
    std::string value_str, key_str;
    for (int i = 0; i < length; ++i)
    {
        key_str.clear();
        if (input[i] == '\n' || input[i] == '\r')
            continue;
        std::unordered_map<std::string, std::function<void(std::string)>>::iterator iter;
        key_str += input[i];
        if ((iter = data_find_map_.find(key_str)) != data_find_map_.end())
        {
            iter->second(value_str);
            value_str.clear();
            continue;
        }
        if ((iter = data_find_map_.find(key_str += input[i + 1])) != data_find_map_.end())
        {
            iter->second(value_str);
            value_str.clear();
            ++i;
        }
        else
        {
            key_str.pop_back();
            value_str.append(key_str);
        }
    }
};
std::vector<pdf::TjData> PdfParserContents::ParserTj(std::string input)
{
    std::vector<pdf::TjData> data_vec;
    pdf::TjData tjdata;
    bool is_contents = true;
    for (auto iter = input.begin(); iter != input.end();)
    {
        if (*iter == '[' || *iter == ']')
            iter = input.erase(iter);
        else
            ++iter;
    }
    for (auto iter = input.begin(); iter != input.end(); ++iter)
    {
        if (*iter == '<' || *iter == '(')
        {
            is_contents = true;
            if (iter != input.begin())
                data_vec.push_back(tjdata);
            tjdata.Clear();
        }
        else if (*iter == '>' || *iter == ')')
        {
            if (*iter == '>')
                tjdata.is_hex = true;
            else
                tjdata.is_hex = false;
            is_contents = false;
        }
        else
        {
            if (is_contents)
                tjdata.contents += *iter;
            else
                tjdata.width += *iter;
        }
    }
    data_vec.push_back(tjdata);
    return data_vec;
};
void PdfParserContents::DealFontData()
{
    auto font_map = child_page_ptr_->GetFontObject();
    for (auto iter = font_map.begin(); iter != font_map.end(); ++iter)
    {
        pdf::FontData fontdata;
        pdf::PdfParser parser;
        fseek(file_ptr_, pdf::ParserSingleton::GetInstance()->GetXref()[iter->second].object_offset_int, SEEK_SET);
        parser.ObjectParser(file_ptr_);
        auto data = pdf::ParserSingleton::GetInstance()->GetDictionary();
        for (auto key_iter = data.begin(); key_iter != data.end(); ++key_iter)
        {
            if (key_iter->first == "Type")
                fontdata.type = key_iter->second;
            else if (key_iter->first == "Subtype")
                fontdata.subtype = key_iter->second;
            else if (key_iter->first == "BaseFont")
                fontdata.baseFont = key_iter->second;
            else if (key_iter->first == "FirstChar")
                fontdata.firstChar = key_iter->second;
            else if (key_iter->first == "LastChar")
                fontdata.lastChar = key_iter->second;
            else if (key_iter->first == "Widths")
                fontdata.widths = key_iter->second;
            else if (key_iter->first == "FontDescriptor")
                fontdata.fontDescriptor = key_iter->second;
            else if (key_iter->first == "ToUnicode")
                fontdata.toUnicode = key_iter->second;
        }
        //toUnicode
        if (fontdata.toUnicode.empty())
            continue;
        int tounicode_obj;
        if (fontdata.toUnicode.find('R') != fontdata.toUnicode.npos)
        {
            tounicode_obj = pdf::GetObjectNumber(fontdata.toUnicode);
        }
        else
        {
            tounicode_obj = atoi(fontdata.toUnicode.c_str());
        }
        fseek(file_ptr_, pdf::ParserSingleton::GetInstance()->GetXref()[tounicode_obj].object_offset_int, SEEK_SET);
        parser.ObjectParser(file_ptr_);
        auto unicode_map = pdf::ParserSingleton::GetInstance()->GetDictionary();
        fontdata.unicode_data.filter = unicode_map["Filter"];
        fontdata.unicode_data.length = atoi(unicode_map["Length"].c_str());
        pdf::PdfUncompress pdf_uncompress;
        unsigned char *dest_ptr = new unsigned char[kUncompressSize];
        size_t dest_length = kUncompressSize;
        if (pdf_uncompress.PdfUncompressData((unsigned char *)pdf::ParserSingleton::GetInstance()->Getstream(), fontdata.unicode_data.length, dest_ptr, dest_length, fontdata.unicode_data.filter))
        {
            fontdata.unicode_data.streamlength = dest_length;
            fontdata.unicode_data.contents_str = (char *)dest_ptr;
        }
        //fontDescriptor
        int font_des_obj;
        if (fontdata.fontDescriptor.find('R') != fontdata.fontDescriptor.npos)
        {
            font_des_obj = pdf::GetObjectNumber(fontdata.fontDescriptor);
        }
        else
        {
            font_des_obj = atoi(fontdata.fontDescriptor.c_str());
        }
        fseek(file_ptr_, pdf::ParserSingleton::GetInstance()->GetXref()[font_des_obj].object_offset_int, SEEK_SET);
        parser.ObjectParser(file_ptr_);
        auto font_des_map = pdf::ParserSingleton::GetInstance()->GetDictionary();
        std::string font_des_value;
        for (auto fontfile_iter = font_des_map.begin(); fontfile_iter != font_des_map.end(); ++fontfile_iter)
        {
            if (fontfile_iter->first.find("FontFile") != fontfile_iter->first.npos)
            {
                font_des_value = fontfile_iter->second;
                break;
            }
        }
        int font_des_sream_obj;
        if (font_des_value.find('R') != font_des_value.npos)
        {
            font_des_sream_obj = pdf::GetObjectNumber(font_des_value);
        }
        else
        {
            font_des_sream_obj = atoi(font_des_value.c_str());
        }
        fseek(file_ptr_, pdf::ParserSingleton::GetInstance()->GetXref()[font_des_sream_obj].object_offset_int, SEEK_SET);
        parser.ObjectParser(file_ptr_);
        auto font_des_stream_map = pdf::ParserSingleton::GetInstance()->GetDictionary();
        fontdata.font_des_data.filter = font_des_stream_map["Filter"];
        fontdata.font_des_data.length = atoi(font_des_stream_map["Length"].c_str());
        unsigned char *dest_stream_ptr = new unsigned char[kUncompressSize];
        size_t dest_stream_length = kUncompressSize;
        if (pdf_uncompress.PdfUncompressData((unsigned char *)pdf::ParserSingleton::GetInstance()->Getstream(), fontdata.font_des_data.length, dest_stream_ptr, dest_stream_length, fontdata.font_des_data.filter))
        {
            fontdata.font_des_data.streamlength = dest_stream_length;
            fontdata.font_des_data.contents_str = (char *)dest_stream_ptr;
            auto finall_data = text_ptr_->GetFontData();
            finall_data.insert(std::make_pair(iter->first, fontdata));
            text_ptr_->SetFontData(finall_data);
        }
        delete[] dest_stream_ptr;
        dest_stream_ptr = nullptr;
        delete[] dest_ptr;
        dest_ptr = nullptr;
        fontdata.unicode_data.contents_str = nullptr;
        fontdata.font_des_data.contents_str = nullptr;
    }
}
void PdfParserContents::DealContentsData()
{
    fseek(file_ptr_, pdf::ParserSingleton::GetInstance()->GetXref()[child_page_ptr_->GetContentsObject()].object_offset_int, SEEK_SET);
    pdf::PdfParser parser;
    parser.ObjectParser(file_ptr_);
    auto iter_map = pdf::ParserSingleton::GetInstance()->GetDictionary();
    pdf::PdfUncompress pdf_uncompress;
    unsigned char *dest_ptr = new unsigned char[kUncompressSize];
    size_t dest_length = kUncompressSize;
    if (pdf_uncompress.PdfUncompressData((unsigned char *)pdf::ParserSingleton::GetInstance()->Getstream(), atoi(iter_map["Length"].c_str()), dest_ptr, dest_length, iter_map["Filter"]))
    {
        ParserContents((char *)dest_ptr, dest_length);
    }
    delete[] dest_ptr;
    dest_ptr = nullptr;
}
} // namespace pdf