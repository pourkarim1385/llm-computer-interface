#include "CaptureLinux.hpp"
#include <fstream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../Models/stb_image_write.h"

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

std::vector<unsigned char> captureScreenshotLinux(ImageFormat& OUT_fmt, ImageFormat fmt,
                                                   const std::string& outTxtPath) {
    Display* display = XOpenDisplay(nullptr);
    Window root = DefaultRootWindow(display);
    XWindowAttributes attrs;
    XGetWindowAttributes(display, root, &attrs);
    int width = attrs.width, height = attrs.height;

    XImage* img = XGetImage(display, root, 0, 0, width, height, AllPlanes, ZPixmap);

    std::vector<unsigned char> rgb(width * height * 3);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned long pixel = XGetPixel(img, x, y);
            int i = (y * width + x) * 3;
            rgb[i]   = (pixel & img->red_mask)   >> 16;
            rgb[i+1] = (pixel & img->green_mask) >> 8;
            rgb[i+2] =  pixel & img->blue_mask;
        }
    }

    XDestroyImage(img);
    XCloseDisplay(display);

    std::vector<unsigned char> raw;
    auto writer = [](void* ctx, void* data, int size) {
        auto* v = static_cast<std::vector<unsigned char>*>(ctx);
        v->insert(v->end(), (unsigned char*)data, (unsigned char*)data + size);
    };

    if (fmt == ImageFormat::PNG)
        stbi_write_png_to_func(writer, &raw, width, height, 3, rgb.data(), width * 3);
    else
        stbi_write_jpg_to_func(writer, &raw, width, height, 3, rgb.data(), 90);

    OUT_fmt = fmt;

    std::string b64 = base64_encode(raw);

    std::ofstream out(outTxtPath, std::ios::out | std::ios::trunc);
    out << b64;
    out.close();

    std::vector<unsigned char> b64Vec(b64.begin(), b64.end());
    return b64Vec;

}
