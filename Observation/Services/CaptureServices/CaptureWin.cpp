#include "CaptureWin.hpp"


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../Models/stb_image_write.h"

#include <windows.h>


#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

static const char b64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static std::string base64_encode(const std::vector<unsigned char>& data) {
    std::string out;
    out.reserve(((data.size() + 2) / 3) * 4);

    size_t i = 0;
    while (i + 3 <= data.size()) {
        unsigned int n = (data[i] << 16) | (data[i+1] << 8) | data[i+2];
        out += b64_table[(n >> 18) & 0x3F];
        out += b64_table[(n >> 12) & 0x3F];
        out += b64_table[(n >> 6)  & 0x3F];
        out += b64_table[n & 0x3F];
        i += 3;
    }

    size_t rem = data.size() - i;
    if (rem == 1) {
        unsigned int n = data[i] << 16;
        out += b64_table[(n >> 18) & 0x3F];
        out += b64_table[(n >> 12) & 0x3F];
        out += "==";
    } else if (rem == 2) {
        unsigned int n = (data[i] << 16) | (data[i+1] << 8);
        out += b64_table[(n >> 18) & 0x3F];
        out += b64_table[(n >> 12) & 0x3F];
        out += "=";
    }

    return out;
}

static void compositeCursor(std::vector<unsigned char>& rgb, int width, int height) {
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

        std::vector<unsigned char> pixels(cw * ch * 4);
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
                unsigned char b = pixels[sIdx], g = pixels[sIdx+1], r = pixels[sIdx+2], a = pixels[sIdx+3];
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

std::vector<unsigned char> captureScreenshotWindows(ImageFormat& OUT_fmt, ImageFormat fmt,
                                                     const std::string& outTxtPath,
                                                     int quality) {
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

    std::vector<unsigned char> bgr(width * height * 3);
    GetDIBits(hMemDC, hBitmap, 0, height, bgr.data(), &bi, DIB_RGB_COLORS);

    SelectObject(hMemDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemDC);
    ReleaseDC(nullptr, hScreenDC);

    std::vector<unsigned char> rgb(bgr.size());
    for (size_t i = 0; i < bgr.size(); i += 3) {
        rgb[i]   = bgr[i+2];
        rgb[i+1] = bgr[i+1];
        rgb[i+2] = bgr[i];
    }

    compositeCursor(rgb, width, height);

    std::vector<unsigned char> raw;
    auto writer = [](void* ctx, void* data, int size) {
        auto* v = static_cast<std::vector<unsigned char>*>(ctx);
        v->insert(v->end(), (unsigned char*)data, (unsigned char*)data + size);
    };

    if (fmt == ImageFormat::PNG)
        stbi_write_png_to_func(writer, &raw, width, height, 3, rgb.data(), width * 3);
    else
        stbi_write_jpg_to_func(writer, &raw, width, height, 3, rgb.data(), quality);

    OUT_fmt = fmt;

    std::string b64 = base64_encode(raw);

    std::ofstream out(outTxtPath, std::ios::out | std::ios::trunc);
    out << b64;
    out.close();

    std::vector<unsigned char> b64Vec(b64.begin(), b64.end());
    return b64Vec;
}
