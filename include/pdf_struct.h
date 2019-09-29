#ifndef _PDF_STRUCT_H
#define _PDF_STRUCT_H
#include <string.h>
#include <unordered_map>
#include <vector>
#ifdef __cplusplus
extern "C"
{
#endif
    namespace pdf
    {
    struct XrefTable
    {
        int object_number_int;
        int object_offset_int;
        int object_use_times_int;
        std::string is_useful_char;
    };
    struct ImageData
    {
        std::vector<double> cm_vec;
        std::string filter;
        std::string sMask;
        std::string colorspace;
        std::string type;
        std::string subtype;
        std::vector<double> positon_vec;
        char *iamge_str;
        int width;
        int height;
        int bitsPerComponent;
        int length;
        int streamlength;
        struct SMask
        {
            std::string filter;
            std::string colorspace;
            std::string type;
            std::string subtype;
            std::string decode;
            char *iamge_str;
            int width;
            int height;
            int bitsPerComponent;
            int length;
            int streamlength;
            SMask() : height(0), length(0), width(0), bitsPerComponent(0), streamlength(0), iamge_str(nullptr){};
            ~SMask()
            {
                if (this->iamge_str != nullptr)
                {
                    delete[] this->iamge_str;
                    this->iamge_str = nullptr;
                }
            }
        } smask_data;
        ImageData() : height(0), length(0), width(0), bitsPerComponent(0), streamlength(0), iamge_str(nullptr){};
        ~ImageData()
        {
            if (this->iamge_str != nullptr)
            {
                delete[] this->iamge_str;
                this->iamge_str = nullptr;
            }
        }
        ImageData(const ImageData &other)
        {
            iamge_str = new char[other.streamlength];
            memcpy(iamge_str, other.iamge_str, other.streamlength);
            cm_vec = other.cm_vec;
            filter = other.filter;
            sMask = other.sMask;
            colorspace = other.colorspace;
            type = other.type;
            subtype = other.subtype;
            positon_vec = other.positon_vec;
            width = other.width;
            height = other.height;
            bitsPerComponent = other.bitsPerComponent;
            length = other.length;
            streamlength = other.streamlength;
            //smask
            this->smask_data.iamge_str = new char[other.smask_data.streamlength];
            memcpy(this->smask_data.iamge_str, other.smask_data.iamge_str, other.smask_data.streamlength);
            this->smask_data.filter = other.smask_data.filter;
            this->smask_data.colorspace = other.smask_data.colorspace;
            this->smask_data.type = other.smask_data.type;
            this->smask_data.subtype = other.smask_data.subtype;
            this->smask_data.decode = other.smask_data.decode;
            this->smask_data.width = other.smask_data.width;
            this->smask_data.height = other.smask_data.height;
            this->smask_data.bitsPerComponent = other.smask_data.bitsPerComponent;
            this->smask_data.length = other.smask_data.length;
            this->smask_data.streamlength = other.smask_data.streamlength;
        }
    };
    //字体信息
    struct FontData
    {
        std::string type;
        std::string subtype;
        std::string baseFont;
        std::string firstChar;
        std::string lastChar;
        std::string widths;
        std::string fontDescriptor;
        std::string toUnicode;
        struct FontDescriptor
        {
            std::string filter;
            char *contents_str;
            int length;
            int streamlength;
            FontDescriptor() : contents_str(nullptr), length(0), streamlength(0)
            {
            }
            ~FontDescriptor()
            {
                if (this->contents_str != nullptr)
                {
                    delete[] this->contents_str;
                    this->contents_str = nullptr;
                }
            }
        } font_des_data;
        struct UnicodeData
        {
            std::string filter;
            char *contents_str;
            int length;
            int streamlength;
            UnicodeData() : contents_str(nullptr), length(0), streamlength(0)
            {
            }
            ~UnicodeData()
            {
                if (this->contents_str != nullptr)
                {
                    delete[] this->contents_str;
                    this->contents_str = nullptr;
                }
            }
        } unicode_data;
        FontData(){};
        FontData(const FontData &other)
        {
            this->unicode_data.contents_str = new char[other.unicode_data.streamlength];
            memcpy(this->unicode_data.contents_str, other.unicode_data.contents_str, other.unicode_data.streamlength);

            this->font_des_data.contents_str = new char[other.font_des_data.streamlength];
            memcpy(this->font_des_data.contents_str, other.font_des_data.contents_str, other.font_des_data.streamlength);
            this->type = other.type;
            this->subtype = other.subtype;
            this->baseFont = other.baseFont;
            this->firstChar = other.firstChar;
            this->lastChar = other.lastChar;
            this->widths = other.widths;
            this->fontDescriptor = other.fontDescriptor;
            this->toUnicode = other.toUnicode;

            this->unicode_data.filter = other.unicode_data.filter;
            this->unicode_data.length = other.unicode_data.length;
            this->unicode_data.streamlength = other.unicode_data.streamlength;

            this->font_des_data.filter = other.font_des_data.filter;
            this->font_des_data.length = other.font_des_data.length;
            this->font_des_data.streamlength = other.font_des_data.streamlength;
        }
        ~FontData()
        {
        }
    };
    struct TjData
    {
        std::string contents;
        std::string width;
        bool is_hex;
        TjData() : is_hex(true){};
        void Clear()
        {
            contents = "";
            width = "";
            is_hex = true;
        }
    };
    struct WordData
    {
        std::vector<double> color;
        std::vector<double> td;
        std::vector<double> tm;
        std::unordered_map<std::string, int> tf;
        std::vector<std::string> tf_order;
        std::vector<std::vector<TjData>> tj;
    };
    struct Point2D
    {
        int x;
        int y;
        Point2D(int x_num, int y_num) : x(x_num), y(y_num){};
        Point2D() : x(0), y(0){};
    };
    struct LineData
    {
        std::vector<double> color;
        double thick;
        pdf::Point2D point_m;
        std::vector<pdf::Point2D> point_l;
        LineData() : thick(0.0){};
    };
    enum CompressType
    {
        kASCIIHexDecode = 0,
        kASCII85Decode,
        kLZWDecode,
        kFlateDecode,
        kRunLengthDecode,
        kCCITTFaxDecode,
        kJBIG2Decode,
        kDCTDecode,
        kJPXDecode,
        kCrypt,
    };
    struct Image
    {
        int colorspace;
        int width;
        int height;
        int alpha;
        Image() : colorspace(0), width(0), height(0), alpha(0){};
    };
    } // namespace pdf
#ifdef __cplusplus
}
#endif
#endif // _PDF_STRUCT_H