#pragma once
#include <vector>
#include <cstdint>

enum class ImageFormat { JPG, PNG };


std::vector<uint8_t> captureScreenshotWindows(ImageFormat fmt = ImageFormat::PNG, int quality = 75);
