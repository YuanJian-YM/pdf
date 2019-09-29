#include "pdf_page.h"
namespace pdf
{
PdfPage::PdfPage(int pagenumber) : page_number_(pagenumber), render_data_(nullptr)
{
    pdf_contents_ptr_ = std::make_shared<PdfParserContents>(pagenumber);
}
PdfPage::~PdfPage()
{
}
void PdfPage::GetCmapPtr(pdf::PdfTextPtr input_ptr)
{
    auto fontdata = input_ptr->GetFontData();
    for (auto iter = fontdata.begin(); iter != fontdata.end(); ++iter)
    {
        pdf::PDFCmapPtr pCmapPtr = pdf::loadCmap(iter->second.unicode_data.contents_str);
        cmap_map_.insert(std::make_pair(iter->first, pCmapPtr));
    }
}
int PdfPage::GetPageSum() const
{
    return pdf::ParserSingleton::GetInstance()->GetPageSum();
}
std::vector<double> PdfPage::GetMediaBox() const
{
    return pdf_contents_ptr_->GetMediaBox();
}
std::string PdfPage::GetText()
{
    std::string out_str;
    pdf::PdfTextPtr textptr = pdf_contents_ptr_->LoadTextData();
    auto fontdata = textptr->GetFontData();
    GetCmapPtr(textptr);
    auto text_data = textptr->GetTextData();
    for (pdf::WordData data : text_data)
    {
        std::unordered_map<std::string, pdf::FontData>::iterator fontiter;
        if (data.tf.size() == 1)
        {
            fontiter = fontdata.find(data.tf.begin()->first);
            if (fontiter == fontdata.end())
                continue;
            if (fontiter->second.subtype != "TrueType" && fontiter->second.subtype != "Type1")
                continue;
            std::unordered_map<std::string, pdf::PDFCmapPtr>::iterator iter;
            iter = cmap_map_.find(data.tf.begin()->first);
            if (iter == cmap_map_.end())
                continue;
            for (int i = 0; i < data.tj.size(); ++i)
            {
                for (pdf::TjData tj : data.tj[i])
                {
                    if (tj.is_hex)
                    {
                        std::string dest_str;
                        pdf::PdfText::ConvertString(iter->second, pdf::PdfText::TransWord(tj.contents), dest_str);
                        out_str.append(dest_str);
                    }
                    else
                    {
                        out_str.append(tj.contents);
                    }
                }
            }
        }
        else
        {
            for (int i = 0; i < data.tf_order.size(); ++i)
            {
                fontiter = fontdata.find(data.tf_order[i]);
                if (fontiter == fontdata.end())
                    continue;
                if (fontiter->second.subtype != "TrueType" && fontiter->second.subtype != "Type1")
                    continue;
                std::unordered_map<std::string, pdf::PDFCmapPtr>::iterator iter;
                iter = cmap_map_.find(data.tf_order[i]);
                if (iter == cmap_map_.end())
                    continue;
                for (int j = 0; j < data.tj[i].size(); ++j)
                {
                    if (data.tj[i][j].is_hex)
                    {
                        std::string dest_str;
                        pdf::PdfText::ConvertString(iter->second, pdf::PdfText::TransWord(data.tj[i][j].contents), dest_str);
                        out_str.append(dest_str);
                    }
                    else
                    {
                        out_str.append(data.tj[i][j].contents);
                    }
                }
            }
        }
    }
    return out_str;
}
bool PdfPage::SaveImageToPng()
{
    pdf::PdfImagePtr imageptr = pdf_contents_ptr_->LoadImageData();
    auto data_vec = imageptr->GetImageData();
    int index = 0;
    for (pdf::ImageData data : data_vec)
    {
        pdf::PdfImage::FuseSmaskImage(data);
        std::string outfile(pdf::ParserSingleton::GetInstance()->GetOutFilePath());
        outfile.append(std::to_string(page_number_));
        outfile.append("页");
        outfile.append(std::to_string(++index));
        outfile.append(".png");
        pdf::Image image;
        image.colorspace = pdf::GetColorspaceBydevice(data.colorspace);
        image.height = data.height;
        image.width = data.width;
        pdf::WriteDataToPng(data.iamge_str, outfile, image);
    }
    return true;
}
bool PdfPage::RenderPdfToPng()
{
    auto mediabox = pdf_contents_ptr_->GetMediaBox();
    int colorspace = pdf::GetColorspaceBydevice(pdf_contents_ptr_->GetPdfPageColorspace());
    int page_width = round(mediabox[2]);
    int page_height = round(mediabox[3]);
    render_data_ = new char[page_width * page_height * colorspace];
    memset(render_data_, 0xFF, page_width * page_height * colorspace);
    //render image data
    pdf::PdfImagePtr imageptr = pdf_contents_ptr_->LoadImageData();
    RenderImageData(colorspace, page_width, page_height, imageptr, render_data_);
    //render text data
    pdf::PdfTextPtr textptr = pdf_contents_ptr_->LoadTextData();
    RenderTextData(colorspace, page_width, page_height, textptr, render_data_);
    RenderLineData(colorspace, page_width, page_height, textptr, render_data_);
    pdf::Image image;
    image.colorspace = colorspace;
    image.height = page_height;
    image.width = page_width;
    std::string outfile(pdf::ParserSingleton::GetInstance()->GetOutFilePath());
    outfile.append(std::to_string(page_number_));
    outfile.append("页");
    outfile.append(".png");
    pdf::WriteDataToPng(render_data_, outfile, image);
    delete[] render_data_;
    render_data_ = nullptr;
}
bool PdfPage::RenderLineData(int colorspace, int page_width, int page_height, const pdf::PdfTextPtr &text_ptr, char *render_data)
{
    auto line_data = text_ptr->GetLineData();
    if (line_data.color.size() != colorspace)
        return false;
    if (line_data.color.size() == 0)
        return false;
    if (line_data.point_l.size() == 0)
        return false;
    if (line_data.thick == 0.0)
        return false;
    int width = line_data.point_l.begin()->x - line_data.point_m.x;
    int height = round(line_data.thick);
    int start_y = page_height - line_data.point_m.y - 1;
    for (int i = 0; i < height; ++i)
    {
        int pos = start_y * page_width * colorspace + colorspace * line_data.point_m.x;
        ++start_y;
        for (int j = 0; j < width; ++j)
        {
            for (int n = 0; n < colorspace; ++n)
            {
                render_data_[pos + colorspace * j + n] = 255 * line_data.color[n];
            }
        }
    }
}

bool PdfPage::RenderImageData(int colorspace, int page_width, int page_height, const pdf::PdfImagePtr &image_ptr, char *render_data)
{
    auto data_vec = image_ptr->GetImageData();
    for (pdf::ImageData data : data_vec)
    {
        pdf::PdfImage::FuseSmaskImage(data);
        int image_colorspace = pdf::GetColorspaceBydevice(data.colorspace);
        if (colorspace != image_colorspace)
        {
            std::cout << "保存图片到png，colorspace不对" << std::endl;
            continue;
        }
        auto cm = data.cm_vec;
        if (cm.size() != 6)
            continue;
        int width = round(cm[0]);
        int height = round(cm[3]);
        char *scale_data = new char[width * height * image_colorspace];
        pdf::PdfImage::ScaleImage(width, height, data, image_colorspace, (unsigned char *)scale_data);
        pdf::Point2D point(round(cm[4]), round(page_height - cm[5] - cm[3] - 1));
        for (int i = 0; i < height; ++i)
        {
            int pos = point.y * page_width * colorspace + colorspace * point.x;
            ++point.y;
            char *temp_ptr = new char[width * colorspace];
            memcpy(temp_ptr, scale_data + i * width * colorspace, width * colorspace);
            for (int j = 0; j < width * colorspace; ++j)
            {
                render_data_[pos + j] = temp_ptr[j];
            }
            delete[] temp_ptr;
        }
        delete[] scale_data;
    }
    return true;
}
std::vector<int> PdfPage::TransHexToNum(const WordData &input, int index)
{
    std::vector<int> symbol_vec;
    for (int i = 0; i < input.tj[index].size(); ++i)
    {
        if (input.tj[index][i].is_hex)
        {
            std::string temp;
            int length = input.tj[index][i].contents.size();
            bool is_add = false;
            if (length % 2 != 0)
                is_add = true;
            for (int j = 0; j < input.tj[index][i].contents.size() - (is_add ? 1 : 0); j += 2)
            {
                char ch = 0;
                ch += pdf::HexToNum(input.tj[index][i].contents[j]) * 16;
                ch += pdf::HexToNum(input.tj[index][i].contents[j + 1]);
                symbol_vec.push_back(ch);
            }
            if (is_add)
            {
                char ch = 0;
                ch += pdf::HexToNum(input.tj[index][i].contents[length - 1]) * 16;
                symbol_vec.push_back(ch);
            }
        }
        else
        {
            for (int j = 0; j < input.tj[index][i].contents.size(); ++j)
            {
                symbol_vec.push_back(input.tj[index][i].contents[j]);
            }
        }
    }
    return symbol_vec;
}
void PdfPage::ChangeFontImageData(unsigned char *src, std::vector<double> color, int height, int width, int colorspace, unsigned char *dest)
{
    unsigned char *dp = dest;
    if (color.size() == colorspace)
    {
        if (color.size() == colorspace)
        {
            unsigned int rgba = 0xFF000000;
            unsigned int mask = 0xFF00FF00;
            unsigned int rb = 0;
            unsigned int ga = 0;

            rgba |= (unsigned int)color[0];
            rgba |= (unsigned int)color[1] << 8;
            rgba |= (unsigned int)color[2] << 16;

            rgba |= 0xFF000000;
            rb = rgba & (mask >> 8);
            ga = (rgba & mask) >> 8;
            for (int i = 0; i < height; ++i)
            {
                for (int j = 0; j < width; ++j)
                {
                    unsigned int ma = *src++;
                    ma += (ma >> 7);
                    if (ma == 0)
                    {
                        dp += colorspace;
                    }
                    else if (ma == 256)
                    {
                        unsigned char *r = (unsigned char *)&rgba;
                        for (int i = 0; i < colorspace; ++i)
                        {
                            *dp++ = *r++;
                        }
                    }
                    else
                    {
                        unsigned int RGBA = 0xFFFFFFFF;
                        unsigned int RB = (RGBA << 8) & mask;
                        unsigned int GA = RGBA & mask;
                        RB += (rb - (RB >> 8)) * ma;
                        GA += (ga - (GA >> 8)) * ma;
                        RB &= mask;
                        GA &= mask;

                        unsigned int rgb = (RB >> 8) | GA;
                        unsigned char *r = (unsigned char *)&rgb;
                        for (int i = 0; i < colorspace; ++i)
                        {
                            *dp++ = *r++;
                        }
                    }
                }
            }
        }
    }
    else
    {
        for (int i = 0; i < height; ++i)
        {
            for (int j = 0; j < width; ++j)
            {
                for (int n; n < colorspace; ++n)
                {
                    *dp++ = 0xFF - *src;
                }
                ++src;
            }
        }
    }
}
bool PdfPage::RenderTextData(int colorspace, int page_width, int page_height, const pdf::PdfTextPtr &text_ptr, char *render_data)
{
    FT_Library library;
    FT_Face face;
    if (FT_Init_FreeType(&library))
        return false;
    cmap_map_.clear();
    GetCmapPtr(text_ptr);
    auto font_vec = text_ptr->GetFontData();
    auto data_vec = text_ptr->GetTextData();
    for (pdf::WordData data : data_vec)
    {
        for (int i = 0; i < data.tf_order.size(); ++i)
        {
            std::unordered_map<std::string, pdf::PDFCmapPtr>::iterator iter;
            iter = cmap_map_.find(data.tf.begin()->first);
            if (iter == cmap_map_.end())
                continue;
            if (FT_New_Memory_Face(library, (unsigned char *)font_vec[data.tf.begin()->first].font_des_data.contents_str, font_vec[data.tf.begin()->first].font_des_data.streamlength, 0, &face))
                continue;
            if (ChooseFace(face, font_vec[data.tf.begin()->first]) == false)
                continue;
            auto cidtogid_vec = GetCidToGid(face);
            if (cidtogid_vec.empty())
                continue;
            if (data.td.size() != 2)
                continue;
            double x, y;
            if (data.tm.size() != 6)
            {
                x = data.td[0];
                y = page_height - data.td[1];
            }
            else
            {
                x = data.tm[4];
                y = page_height - data.tm[5];
            }

            auto font_iter = data.tf.find(data.tf_order[i]);
            if (font_iter == data.tf.end())
                continue;
            if (FT_Set_Char_Size(face, font_iter->second * 64, font_iter->second * 64, 72, 72))
            {
                continue;
            }
            //calculate data
            auto color = data.color;
            std::vector<int> symbol_vec;
            symbol_vec = TransHexToNum(data, i);
            for (auto symbol : symbol_vec)
            {
                if (symbol < 0 || symbol > cidtogid_vec.size())
                    continue;
                FT_Glyph glyph;
                int jj = symbol;

                if (FT_Load_Glyph(face, cidtogid_vec[symbol], FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING))
                    continue;
                if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL))
                    continue;
                if (FT_Get_Glyph(face->glyph, &glyph))
                    continue;
                FT_BBox bbox;
                FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_TRUNCATE, &bbox);
                FT_Bitmap bitmap = face->glyph->bitmap;
                unsigned char *src_word_ptr = bitmap.buffer;
                unsigned char *src = src_word_ptr;
                unsigned char *dest = new unsigned char[bitmap.rows * bitmap.width * colorspace];
                unsigned char *dest_temp = dest;
                memset(dest, 0xFF, bitmap.rows * bitmap.width * colorspace);
                ChangeFontImageData(src, color, bitmap.rows, bitmap.width, colorspace, dest_temp);
                //融合到整个图片中
                pdf::Point2D point(round(x), round(y - bitmap.rows - bbox.yMin));
                for (int i = 0; i < bitmap.rows; ++i)
                {
                    int pos = point.y * page_width * colorspace + colorspace * point.x;
                    ++point.y;
                    char *temp_ptr = new char[bitmap.width * colorspace];
                    memcpy(temp_ptr, dest_temp + i * bitmap.width * colorspace, bitmap.width * colorspace);
                    for (int j = 0; j < bitmap.width * colorspace; ++j)
                    {
                        render_data_[pos + j] = temp_ptr[j];
                    }
                    delete[] temp_ptr;
                }
                x += round((double)face->glyph->advance.x / 64);
                delete[] src_word_ptr;
                src_word_ptr = nullptr;
                delete[] dest;
                dest = nullptr;
            }
            FT_Done_Face(face);
            face = nullptr;
        }
    }
    FT_Done_FreeType(library);
    library = nullptr;
}
bool PdfPage::ChooseFace(FT_Face &face, const pdf::FontData &fontdata)
{
    FT_CharMap cmap;
    if (fontdata.subtype == "Type1")
    {
        cmap = SelectType1Cmap(face);
    }
    else if (fontdata.subtype == "TrueType")
    {
        cmap = SelectTrueTypeCmap(face, 1);
    }
    if (cmap)
    {
        if (FT_Set_Charmap(face, cmap))
        {
            return false;
        }
    }
    return true;
}
FT_CharMap PdfPage::SelectType1Cmap(FT_Face face)
{
    int i;
    for (i = 0; i < face->num_charmaps; i++)
        if (face->charmaps[i]->platform_id == 7)
            return face->charmaps[i];
    if (face->num_charmaps > 0)
        return face->charmaps[0];
    return nullptr;
}
FT_CharMap PdfPage::SelectTrueTypeCmap(FT_Face face, int symbolic)
{
    int i;

    /* First look for a Microsoft symbolic cmap, if applicable */
    if (symbolic)
    {
        for (i = 0; i < face->num_charmaps; i++)
            if (face->charmaps[i]->platform_id == 3 && face->charmaps[i]->encoding_id == 0)
                return face->charmaps[i];
    }

    /* Then look for a Microsoft Unicode cmap */
    for (i = 0; i < face->num_charmaps; i++)
        if (face->charmaps[i]->platform_id == 3 && face->charmaps[i]->encoding_id == 1)
            if (FT_Get_CMap_Format(face->charmaps[i]) != -1)
                return face->charmaps[i];

    /* Finally look for an Apple MacRoman cmap */
    for (i = 0; i < face->num_charmaps; i++)
        if (face->charmaps[i]->platform_id == 1 && face->charmaps[i]->encoding_id == 0)
            if (FT_Get_CMap_Format(face->charmaps[i]) != -1)
                return face->charmaps[i];

    if (face->num_charmaps > 0)
        if (FT_Get_CMap_Format(face->charmaps[0]) != -1)
            return face->charmaps[0];
    return nullptr;
}
std::vector<unsigned int> PdfPage::GetCidToGid(FT_Face face)
{
    std::vector<unsigned int> cidtogid_vec;
    for (int cid = 0; cid < 256; ++cid)
    {
        int gid = FT_Get_Char_Index(face, cid);
        if (gid == 0)
            gid = FT_Get_Char_Index(face, 0xf000 + cid);

        /* some chinese fonts only ship the similarly looking 0x2026 */
        if (gid == 0 && cid == 0x22ef)
            gid = FT_Get_Char_Index(face, 0x2026);
        cidtogid_vec.push_back(gid);
    }
    return cidtogid_vec;
}
} //namespace pdf