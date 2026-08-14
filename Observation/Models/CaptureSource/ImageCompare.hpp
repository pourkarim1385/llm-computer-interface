#pragma once
#include <string>
#include <filesystem>
#include <vector>
#include <regex>
#include <iostream>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <bitset>

bool cleanupDuplicateScreenshots(const std::string& dir, double thresholdPercent = 20.0);
