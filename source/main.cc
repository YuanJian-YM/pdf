/*
 * @Author: mikey.yuanjian 
 * @Date: 2019-06-15 15:34:54 
 * @Last Modified by: mikey.yuanjian
 * @Last Modified time: 2019-06-15 19:32:17
 */
#include <gflags/gflags.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <glog/logging.h>
#include "pdf_base_init.h"
#include "pdf_basicdata_singleton.h"
#include "pdf_page.h"
//初始化
DEFINE_string(input_file, "", "input file");
DEFINE_string(output_path, "", "output file path");
DEFINE_uint64(page_number, 0, "page number");
//功能定义
DEFINE_bool(get_pdf_count, false, "get pdf page count");
DEFINE_bool(get_page_text, false, "get padf page text");
DEFINE_bool(get_page_image, false, "get pdf page images");
DEFINE_bool(get_render_image, false, "render pdf to image");
DEFINE_bool(get_page_rect, false, "get pdf page rect");
int main(int argc, char *argv[])
{
    clock_t start, end;
    start = clock();
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    // init glog
    google::InitGoogleLogging("pdf");
    google::SetLogDestination(google::INFO, "log_info.txt");
    google::SetLogDestination(google::WARNING, "log_warning.txt");
    google::SetLogDestination(google::ERROR, "log_error.txt");
    LOG(INFO) << "glog init success!";
    pdf::PdfBaseInit pdfinit(FLAGS_input_file, FLAGS_output_path);
    pdfinit.Init();
    if (FLAGS_get_pdf_count)
    {
        std::cout << "页面数量:" << pdf::ParserSingleton::GetInstance()->GetPageSum() << std::endl;
    }
    for (int i = 1; i < pdf::ParserSingleton::GetInstance()->GetPageSum(); ++i)
    {
        pdf::PdfPage page(i);
        if (FLAGS_get_page_rect)
        {
            vector<double> vecMediaBox = page.GetMediaBox();
            std::cout << "mediabox:" << std::endl;
            for (int i = 0; i < vecMediaBox.size(); ++i)
            {
                std::cout << vecMediaBox[i] << " ";
            }
            std::cout << std::endl;
        }
        if (FLAGS_get_page_text)
        {
            std::cout << page.GetText() << std::endl;
        }
        if (FLAGS_get_page_image)
        {
            page.SaveImageToPng();
        }
        if (FLAGS_get_render_image)
        {
            page.RenderPdfToPng();
        }
    }
    pdf::ParserSingleton::GetInstance()->Destroy();
    pdfinit.PdfClose();
    gflags::ShutDownCommandLineFlags();
    google::ShutdownGoogleLogging();
    end = clock();
    std::cout << "程序运行时间" << (double)(end - start) / CLOCKS_PER_SEC << std::endl;
    return 0;
}