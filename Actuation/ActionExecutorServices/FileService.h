#pragma once

#include <filesystem>
#include <string>
#include <mutex>
#include <fstream>
#include <stdexcept>
#include <vector>
#include "../Actions.h"

enum class FileErrorCode {
    NOT_FOUND,
    ALREADY_EXISTS,
    CANNOT_OPERATE,
    SYSTEM_ERROR
};

class FileServiceException : public std::runtime_error {
public:
    FileServiceException(FileErrorCode code, const std::string& message)
            : std::runtime_error(message), m_code(code) {}

    FileErrorCode getCode() const { return m_code; }

private:
    FileErrorCode m_code;
};


class FileService {
public:
    // Delete copy constructor and assignment operator to enforce Singleton
    FileService(const FileService&) = delete;
    FileService& operator=(const FileService&) = delete;

    // Singleton access point
    static FileService& getInstance();

    // Action Execution Methods
    void createFile(const std::filesystem::path& path, const std::string& text);
    void writeFile(const std::filesystem::path& path, const std::string& text);
    void appendFile(const std::filesystem::path& path, const std::string& text);
    void insertFile(const std::filesystem::path& path, int position, const std::string& text);

    void deleteFile(const std::filesystem::path& path);
    void renameFile(const std::filesystem::path& path, const std::filesystem::path& new_path);
    void copyFile(const std::filesystem::path& path, const std::filesystem::path& destination);
    void moveFile(const std::filesystem::path& path, const std::filesystem::path& destination);


    // Directory Management Methods
    void createDirectory(const std::filesystem::path& path);
    void copyDirectory(const std::filesystem::path& path, const std::filesystem::path& destination);
    void moveDirectory(const std::filesystem::path& path, const std::filesystem::path& destination);
    void deleteDirectory(const std::filesystem::path& path);


    /*
     * {
    "action": "apply_diff",
        "file": "main.cpp",
        "changes": [
            { "search": "int x = 0;", "replace": "int x = 10;" },
            { "search": "return x;", "replace": "return x * 2;" }
          ]
      }
     * */
    void applyDiff(const std::filesystem::path& path, const std::vector<Actions::ReplaceBlock>& changes);

    void editFile(const std::filesystem::path& path, const std::vector<Actions::TextEdit>& edits);

private:
    // Private constructor for Singleton
    FileService() = default;
    ~FileService() = default;

    // Global mutex for thread safety across the service
    std::mutex m_mutex;
};