#pragma once

#include <filesystem>
#include <string>
#include <mutex>
#include <fstream>
#include <stdexcept>

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

private:
    // Private constructor for Singleton
    FileService() = default;
    ~FileService() = default;

    // Global mutex for thread safety across the service
    std::mutex m_mutex;
};