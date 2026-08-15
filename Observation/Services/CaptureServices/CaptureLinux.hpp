#pragma once

#include <iostream>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <vector>
#include <cstdint>
#include <cstring>
#include <X11/extensions/Xfixes.h>

enum class ImageFormat { PNG, JPG };

std::vector<uint8_t> captureScreenshotLinux(ImageFormat fmt = ImageFormat::PNG);
