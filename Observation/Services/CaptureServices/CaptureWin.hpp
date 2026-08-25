#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include "../../Public.h"

std::vector<uint8_t> captureScreenshotWindows(ImageFormat& OUT_fmt, ImageFormat fmt = ImageFormat::PNG, int quality = 75);
static void compositeCursor(std::vector<unsigned char>& rgb, int width, int height);
static std::string base64_encode(const std::vector<unsigned char>& data);
