#pragma once
#include <vector>
#include <cstdint>
#include "../../Public.h"

std::vector<uint8_t> captureScreenshotWindows(ImageFormat& OUT_fmt, ImageFormat fmt = ImageFormat::PNG, int quality = 75);
