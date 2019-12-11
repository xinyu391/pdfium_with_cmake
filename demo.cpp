#include "fpdfview.h"
#include <iostream>
#include <string>
#include <string.h>
// #include "opencv2/opencv.hpp"
const char* icudt64_dat ="/data2/jesse/work/pdfium/repo/icudtl.dat2";
int main(int argc, char* argv[]) {

    // FPDF_InitLibrary();
    const char* fonts[] = {
        "../fonts/",
        NULL
    };
    FPDF_LIBRARY_CONFIG config;
    config.m_pUserFontPaths = fonts;
    config.version = 2;
    config.m_pIsolate = NULL;
    config.m_v8EmbedderSlot = 0;
    FPDF_InitLibraryWithConfig(&config);
    char* path = "../sample/test.pdf";
    if (argc > 1) {
        path = argv[1];
    }

    FPDF_DOCUMENT doc = FPDF_LoadDocument(path, 0);
    if (doc) {
        int page_count = FPDF_GetPageCount(doc);
        std::cout << "page count " << page_count << std::endl;
        for (int p = 0; p < page_count; p++) {
            FPDF_PAGE page = FPDF_LoadPage(doc, p);
            if (page) {
                // 物理尺寸 每英寸72点
                double pwidth = FPDF_GetPageWidth(page);
                double pheight = FPDF_GetPageHeight(page);
                // 150ppi 下的像素尺寸
                int width = (int)(pwidth)*150 / 72;
                int height = (int)(pheight)*150 / 72;
                std::cout << "\t" << p << " (" << width << "," << height << ")" << std::endl;
                // BGRx/BGRA 4通道
                FPDF_BITMAP bmp = FPDFBitmap_Create(width, height, 0);
                // BGR 3通道
                //  FPDF_BITMAP bmp = FPDFBitmap_CreateEx(width, height, FPDFBitmap_BGR,NULL,0);
                uint8_t* ptr = (uint8_t*)FPDFBitmap_GetBuffer(bmp);
                //设置画板白色背景
                memset(ptr,255,width*height*4);
                FPDF_RenderPageBitmap(bmp, page, 0, 0, width, height, 0, 0);
                std::string name = "page" + std::to_string(p) + ".ppm";
                FILE* fp = fopen(name.c_str(), "wb");
                fprintf(fp, "P6\n%d %d\n255\n", width, height);

                uint8_t* dst = (uint8_t*)malloc(width * height * 3);
                uint8_t* dptr = dst;
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {// BGR->RGB
                        *dst++ = (ptr[2]);// * ptr[3] + 255 * (255 - ptr[3])) / 255;
                        *dst++ = (ptr[1]);// * ptr[3] + 255 * (255 - ptr[3])) / 255;
                        *dst++ = (ptr[0]);// * ptr[3] + 255 * (255 - ptr[3])) / 255;
                        ptr += 4;
                    }
                }
                fwrite(dptr, width * 3, height, fp);
                fclose(fp);
                free(dptr);
                FPDFBitmap_Destroy(bmp);
                FPDF_ClosePage(page);
            }
        }
        FPDF_CloseDocument(doc);
    }

    FPDF_DestroyLibrary();
    return 0;
}