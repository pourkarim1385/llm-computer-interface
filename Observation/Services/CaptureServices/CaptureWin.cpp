#include "CaptureWin.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../Models/stb_image_write.h"

#include <windows.h>
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

static void compositeCursor(std::vector<uint8_t>& rgb, int width, int height) {
    CURSORINFO ci{};
    ci.cbSize = sizeof(ci);
    if (!GetCursorInfo(&ci) || !(ci.flags & CURSOR_SHOWING)) return;

    ICONINFO ii{};
    if (!GetIconInfo(ci.hCursor, &ii)) return;

    BITMAP bmColor{};
    bool hasColor = ii.hbmColor && GetObject(ii.hbmColor, sizeof(bmColor), &bmColor);

    if (hasColor) {
        int cw = bmColor.bmWidth;
        int ch = bmColor.bmHeight;

        BITMAPINFO bi{};
        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth = cw;
        bi.bmiHeader.biHeight = -ch;
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 32;
        bi.bmiHeader.biCompression = BI_RGB;

        std::vector<uint8_t> pixels(cw * ch * 4);
        HDC hdc = GetDC(nullptr);
        GetDIBits(hdc, ii.hbmColor, 0, ch, pixels.data(), &bi, DIB_RGB_COLORS);
        ReleaseDC(nullptr, hdc);

        int cx = ci.ptScreenPos.x - (int)ii.xHotspot;
        int cy = ci.ptScreenPos.y - (int)ii.yHotspot;

        for (int j = 0; j < ch; j++) {
            for (int i = 0; i < cw; i++) {
                int px = cx + i, py = cy + j;
                if (px < 0 || py < 0 || px >= width || py >= height) continue;

                int sIdx = (j * cw + i) * 4;
                uint8_t b = pixels[sIdx], g = pixels[sIdx+1], r = pixels[sIdx+2], a = pixels[sIdx+3];
                if (!a) continue;

                int dIdx = (py * width + px) * 3;
                rgb[dIdx]   = (r * a + rgb[dIdx]   * (255 - a)) / 255;
                rgb[dIdx+1] = (g * a + rgb[dIdx+1] * (255 - a)) / 255;
                rgb[dIdx+2] = (b * a + rgb[dIdx+2] * (255 - a)) / 255;
            }
        }
    }

    if (ii.hbmColor) DeleteObject(ii.hbmColor);
    if (ii.hbmMask)  DeleteObject(ii.hbmMask);
}

std::vector<uint8_t> captureScreenshotWindows(ImageFormat fmt, int quality) {
    int width  = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    HDC hScreenDC = GetDC(nullptr);
    HDC hMemDC = CreateCompatibleDC(hScreenDC);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

    BitBlt(hMemDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY | CAPTUREBLT);

    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = width;
    bi.bmiHeader.biHeight = -height;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biCompression = BI_RGB;

    std::vector<uint8_t> bgr(width * height * 3);
    GetDIBits(hMemDC, hBitmap, 0, height, bgr.data(), &bi, DIB_RGB_COLORS);

    SelectObject(hMemDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemDC);
    ReleaseDC(nullptr, hScreenDC);

    std::vector<uint8_t> rgb(bgr.size());
    for (size_t i = 0; i < bgr.size(); i += 3) {
        rgb[i]   = bgr[i+2];
        rgb[i+1] = bgr[i+1];
        rgb[i+2] = bgr[i];
    }

    compositeCursor(rgb, width, height);

    auto writer = [](void* ctx, void* data, int size) {
        auto* v = static_cast<std::vector<uint8_t>*>(ctx);
        v->insert(v->end(), (uint8_t*)data, (uint8_t*)data + size);
    };

    std::vector<uint8_t> out;
    if (fmt == ImageFormat::JPG) {
        stbi_write_jpg_to_func(writer, &out, width, height, 3, rgb.data(), quality);
    } else {
        stbi_write_png_compression_level = 9; 
        stbi_write_force_png_filter = 0;      
        stbi_write_png_to_func(writer, &out, width, height, 3, rgb.data(), width * 3);
    }
    return out;
}
