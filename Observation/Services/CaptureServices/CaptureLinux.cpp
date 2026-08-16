#include "CaptureLinux.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../Models/stb_image_write.h"

std::vector<unsigned char> captureScreenshotLinux(ImageFormat fmt) {
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

    std::vector<unsigned char> out;
    auto writer = [](void* ctx, void* data, int size) {
        auto* v = static_cast<std::vector<unsigned char>*>(ctx);
        v->insert(v->end(), (unsigned char*)data, (unsigned char*)data + size);
    };

    if (fmt == ImageFormat::PNG)
        stbi_write_png_to_func(writer, &out, width, height, 3, rgb.data(), width * 3);
    else
        stbi_write_jpg_to_func(writer, &out, width, height, 3, rgb.data(), 90);

    return out;
}
