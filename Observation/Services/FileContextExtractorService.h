#ifndef ACCESSIBILITYSERVICE_FILECONTEXTEXTRACTORSERVICE_H
#define ACCESSIBILITYSERVICE_FILECONTEXTEXTRACTORSERVICE_H
#include "../Models/FileContextState.h"

class fileIncludeFilter {
private:
    std::string filterName;
    std::vector<std::string> excludesExtension;
    std::vector<std::string> excludesName;

public:
    fileIncludeFilter();
    explicit fileIncludeFilter(const std::string& name) : filterName(name) {};
    void addExcludeName(std::string name);
    void addExcludeExtension(std::string extension);
    bool isIncludedExtension(std::string extension);
    bool isIncludedName(std::string name);
};

class FileContextException : public std::runtime_error {
public:
    explicit FileContextException(const std::string& message)
        : std::runtime_error("FileContext Error: " + message) {}
};

class FileContextExtractorService {
    private:
    FileContextExtractorService() = default;

    bool isValidPath(const std::string& path);
    FileType typeDetection(const std::string& path);
    std::string extensionDetection(const std::string& path);
    std::vector<FileSystemEntry> neighboursListing(const std::string& path);
    std::vector<FileSystemEntry> entriesTreeListing(const std::string& path);

    bool filter(FileSystemEntry& file, fileIncludeFilter targetFilter = fileIncludeFilter());

    public:
    static FileContextExtractorService& getInstance();
    FileContextExtractorService(const FileContextExtractorService&) = delete;
    FileContextExtractorService& operator=(const FileContextExtractorService&) = delete;
    FileContextState getCurrentState(const std::string& filePath, const fileIncludeFilter targetFilter = fileIncludeFilter());
};

#endif //ACCESSIBILITYSERVICE_FILECONTEXTEXTRACTORSERVICE_H