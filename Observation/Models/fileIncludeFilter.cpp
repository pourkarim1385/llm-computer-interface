#include "fileIncludeFilter.h"
#include <algorithm>

void fileIncludeFilter::addExcludeName(std::string name) {
    excludesName.push_back(name);
}

void fileIncludeFilter::addExcludeExtension(std::string extension) {
    excludesExtension.push_back(extension);
}

bool fileIncludeFilter::isIncludedExtension(std::string extension) {
    return std::find(excludesExtension.begin(), excludesExtension.end(), extension) == excludesExtension.end();
}

bool fileIncludeFilter::isIncludedName(std::string name) {
    return std::find(excludesName.begin(), excludesName.end(), name) == excludesName.end();
}
