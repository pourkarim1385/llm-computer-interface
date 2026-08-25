#include "FileService.h"
#include <sstream>

FileService& FileService::getInstance() {
    static FileService instance;
    return instance;
}

void FileService::createFile(const std::filesystem::path& path, const std::string& text) {
    std::lock_guard<std::mutex> lock(m_mutex);

    // Prevent overwriting if it already exists
    if (std::filesystem::exists(path)) {
        throw FileServiceException(FileErrorCode::ALREADY_EXISTS,
                                   "createFile failed: File already exists at " + path.string());
    }

    std::ofstream file(path);
    if (!file.is_open()) {
        throw FileServiceException(FileErrorCode::CANNOT_OPERATE,
                                   "createFile failed: Could not open file for writing at " + path.string());
    }

    file << text;
}

void FileService::writeFile(const std::filesystem::path& path, const std::string& text) {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::ofstream file(path, std::ios::trunc);
    if (!file.is_open()) {
        throw FileServiceException(FileErrorCode::CANNOT_OPERATE,
                                   "writeFile failed: Could not open file at " + path.string());
    }

    file << text;
}

void FileService::appendFile(const std::filesystem::path& path, const std::string& text) {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::ofstream file(path, std::ios::app);
    if (!file.is_open()) {
        throw FileServiceException(FileErrorCode::CANNOT_OPERATE,
                                   "appendFile failed: Could not open file at " + path.string());
    }

    file << text;
}

void FileService::insertFile(const std::filesystem::path& path, int position, const std::string& text) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!std::filesystem::exists(path)) {
        throw FileServiceException(FileErrorCode::NOT_FOUND,
                                   "insertFile failed: File does not exist at " + path.string());
    }

    std::string current_contents;
    {
        std::ifstream infile(path);
        if (!infile.is_open()) {
            throw FileServiceException(FileErrorCode::CANNOT_OPERATE,
                                       "insertFile failed: Could not open file for reading at " + path.string());
        }
        std::ostringstream ss;
        ss << infile.rdbuf();
        current_contents = ss.str();
    }

    if (position < 0) {
        position = 0;
    } else if (position > static_cast<int>(current_contents.length())) {
        position = current_contents.length();
    }

    current_contents.insert(position, text);

    {
        std::ofstream outfile(path, std::ios::trunc);
        if (!outfile.is_open()) {
            throw FileServiceException(FileErrorCode::CANNOT_OPERATE,
                                       "insertFile failed: Could not open file for writing at " + path.string());
        }
        outfile << current_contents;
    }
}

void FileService::deleteFile(const std::filesystem::path& path) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!std::filesystem::exists(path)) {
        throw FileServiceException(FileErrorCode::NOT_FOUND,
                                   "deleteFile failed: File does not exist at " + path.string());
    }

    try {
        std::filesystem::remove(path);
    } catch (const std::filesystem::filesystem_error& e) {
        throw FileServiceException(FileErrorCode::SYSTEM_ERROR,
                                   std::string("deleteFile failed: ") + e.what());
    }
}

void FileService::renameFile(const std::filesystem::path& path, const std::filesystem::path& new_path) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!std::filesystem::exists(path)) {
        throw FileServiceException(FileErrorCode::NOT_FOUND,
                                   "renameFile failed: Source file does not exist at " + path.string());
    }

    try {
        std::filesystem::rename(path, new_path);
    } catch (const std::filesystem::filesystem_error& e) {
        throw FileServiceException(FileErrorCode::SYSTEM_ERROR,
                                   std::string("renameFile failed: ") + e.what());
    }
}

void FileService::copyFile(const std::filesystem::path& path, const std::filesystem::path& destination) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!std::filesystem::exists(path)) {
        throw FileServiceException(FileErrorCode::NOT_FOUND,
                                   "copyFile failed: Source file does not exist at " + path.string());
    }

    try {
        std::filesystem::copy(path, destination, std::filesystem::copy_options::overwrite_existing);
    } catch (const std::filesystem::filesystem_error& e) {
        throw FileServiceException(FileErrorCode::SYSTEM_ERROR,
                                   std::string("copyFile failed: ") + e.what());
    }
}

void FileService::moveFile(const std::filesystem::path& path, const std::filesystem::path& destination) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!std::filesystem::exists(path)) {
        throw FileServiceException(FileErrorCode::NOT_FOUND,
                                   "moveFile failed: Source file does not exist at " + path.string());
    }

    try {
        std::filesystem::rename(path, destination);
    } catch (const std::filesystem::filesystem_error& e) {
        throw FileServiceException(FileErrorCode::SYSTEM_ERROR,
                                   std::string("moveFile failed: ") + e.what());
    }
}