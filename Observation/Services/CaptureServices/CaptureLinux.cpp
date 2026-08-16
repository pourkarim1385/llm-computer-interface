#include "CaptureLinux.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../Models/stb_image_write.h"

std::vector<uint8_t> captureScreenshotLinux(ImageFormat& OUT_fmt, ImageFormat fmt) {
    Display* display = XOpenDisplay(nullptr);
    if (!display) return {};

    Window root = DefaultRootWindow(display);
    XWindowAttributes attrs;
    XGetWindowAttributes(display, root, &attrs);
    int width = attrs.width, height = attrs.height;

    XImage* img = XGetImage(display, root, 0, 0, width, height, AllPlanes, ZPixmap);
    if (!img) { XCloseDisplay(display); return {}; }

    std::vector<uint8_t> rgb(width * height * 3);
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++) {
            unsigned long pixel = XGetPixel(img, x, y);
            int i = (y * width + x) * 3;
            rgb[i]   = (pixel & img->red_mask)   >> 16;
            rgb[i+1] = (pixel & img->green_mask) >> 8;
            rgb[i+2] = (pixel & img->blue_mask);
        }
    XDestroyImage(img);

    XFixesCursorImage* cursor = XFixesGetCursorImage(display);
    if (cursor) {
        int cx = cursor->x - cursor->xhot, cy = cursor->y - cursor->yhot;
        for (int j = 0; j < (int)cursor->height; j++)
            for (int i = 0; i < (int)cursor->width; i++) {
                int px = cx + i, py = cy + j;
                if (px < 0 || py < 0 || px >= width || py >= height) continue;
                unsigned long argb = cursor->pixels[j * cursor->width + i];
                uint8_t a = (argb >> 24) & 0xFF;
                if (!a) continue;
                uint8_t r = (argb >> 16) & 0xFF, g = (argb >> 8) & 0xFF, b = argb & 0xFF;
                int idx = (py * width + px) * 3;
                rgb[idx]   = (r * a + rgb[idx]   * (255 - a)) / 255;
                rgb[idx+1] = (g * a + rgb[idx+1] * (255 - a)) / 255;
                rgb[idx+2] = (b * a + rgb[idx+2] * (255 - a)) / 255;
            }
        XFree(cursor);
    }
    XCloseDisplay(display);

    auto writer = [](void* ctx, void* data, int size) {
        auto* v = static_cast<std::vector<uint8_t>*>(ctx);
        v->insert(v->end(), (uint8_t*)data, (uint8_t*)data + size);
    };

    std::vector<uint8_t> out;
    if (fmt == ImageFormat::JPG)
        stbi_write_jpg_to_func(writer, &out, width, height, 3, rgb.data(), 90);
    else
        stbi_write_png_to_func(writer, &out, width, height, 3, rgb.data(), width * 3);

    OUT_fmt = fmt;
    return out;
}
