#pragma once

#include <iostream>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <vector>
#include <cstdint>
#include <cstring>
#include <X11/extensions/Xfixes.h>
#include "../../Public.h"

std::vector<unsigned char> captureScreenshotLinux(ImageFormat& OUT_fmt, ImageFormat fmt,
                                                   const std::string& outTxtPath);
static std::string base64_encode(const std::vector<unsigned char>& data);