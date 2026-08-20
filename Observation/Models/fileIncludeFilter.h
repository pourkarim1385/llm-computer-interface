#ifndef ACCESSIBILITYSERVICE_FILEINCLUDEFILTER_H
#define ACCESSIBILITYSERVICE_FILEINCLUDEFILTER_H
#include <vector>
#include <string>

class fileIncludeFilter {
private:
    std::string filterName;
    std::vector<std::string> excludesExtension;
    std::vector<std::string> excludesName;

public:
    fileIncludeFilter() = default;
    explicit fileIncludeFilter(const std::string& name) : filterName(name) {};
    void addExcludeName(std::string name);
    void addExcludeExtension(std::string extension);
    bool isIncludedExtension(std::string extension);
    bool isIncludedName(std::string name);
};


#endif //ACCESSIBILITYSERVICE_FILEINCLUDEFILTER_H
