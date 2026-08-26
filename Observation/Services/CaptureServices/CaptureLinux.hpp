#pragma once

#include <iostream>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <vector>
#include <cstdint>
#include <cstring>
#include <X11/extensions/Xfixes.h>
#include "../../Public.h"

std::vector<uint8_t> captureScreenshotLinux(ImageFormat& OUT_fmt, ImageFormat fmt = ImageFormat::PNG);
