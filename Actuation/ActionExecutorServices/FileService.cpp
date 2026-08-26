#include "FileService.h"
#include <sstream>
#include <algorithm>

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

void FileService::createDirectory(const std::filesystem::path& path) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (std::filesystem::exists(path)) {
        throw FileServiceException(FileErrorCode::ALREADY_EXISTS,
                                   "createDirectory failed: Path already exists at " + path.string());
    }

    try {
        // create_directories creates parent folders if they don't exist yet
        if (!std::filesystem::create_directories(path)) {
            throw FileServiceException(FileErrorCode::CANNOT_OPERATE,
                                       "createDirectory failed: System returned false for " + path.string());
        }
    } catch (const std::filesystem::filesystem_error& e) {
        throw FileServiceException(FileErrorCode::SYSTEM_ERROR,
                                   std::string("createDirectory failed: ") + e.what());
    }
}

void FileService::copyDirectory(const std::filesystem::path& path, const std::filesystem::path& destination) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
        throw FileServiceException(FileErrorCode::NOT_FOUND,
                                   "copyDirectory failed: Source directory does not exist at " + path.string());
    }

    try {
        // recursive flag copies everything inside the directory
        std::filesystem::copy(path, destination,
                              std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
    } catch (const std::filesystem::filesystem_error& e) {
        throw FileServiceException(FileErrorCode::SYSTEM_ERROR,
                                   std::string("copyDirectory failed: ") + e.what());
    }
}

void FileService::moveDirectory(const std::filesystem::path& path, const std::filesystem::path& destination) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
        throw FileServiceException(FileErrorCode::NOT_FOUND,
                                   "moveDirectory failed: Source directory does not exist at " + path.string());
    }

    try {
        std::filesystem::rename(path, destination);
    } catch (const std::filesystem::filesystem_error& e) {
        throw FileServiceException(FileErrorCode::SYSTEM_ERROR,
                                   std::string("moveDirectory failed: ") + e.what());
    }
}

void FileService::deleteDirectory(const std::filesystem::path& path) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!std::filesystem::exists(path)) {
        throw FileServiceException(FileErrorCode::NOT_FOUND,
                                   "deleteDirectory failed: Directory does not exist at " + path.string());
    }

    if (!std::filesystem::is_directory(path)) {
        throw FileServiceException(FileErrorCode::CANNOT_OPERATE,
                                   "deleteDirectory failed: Path is a file, not a directory at " + path.string());
    }

    try {
        // remove_all recursively deletes the directory and all of its contents
        std::filesystem::remove_all(path);
    } catch (const std::filesystem::filesystem_error& e) {
        throw FileServiceException(FileErrorCode::SYSTEM_ERROR,
                                   std::string("deleteDirectory failed: ") + e.what());
    }
}

void FileService::applyDiff(const std::filesystem::path& path, const std::vector<Actions::ReplaceBlock>& changes) {
    std::lock_guard<std::mutex> lock(m_mutex);

    // 1. Validate file exists
    if (!std::filesystem::exists(path)) {
        throw FileServiceException(FileErrorCode::NOT_FOUND,
                                   "applyDiff failed: File does not exist at " + path.string());
    }

    // 2. Read existing contents into memory
    std::string current_contents;
    {
        std::ifstream infile(path);
        if (!infile.is_open()) {
            throw FileServiceException(FileErrorCode::CANNOT_OPERATE,
                                       "applyDiff failed: Could not open file for reading at " + path.string());
        }
        std::ostringstream ss;
        ss << infile.rdbuf();
        current_contents = ss.str();
    } // infile closed

    // 3. Apply all changes sequentially in memory
    for (size_t i = 0; i < changes.size(); ++i) {
        const auto& change = changes[i];

        size_t pos = current_contents.find(change.search_text);
        if (pos == std::string::npos) {
            // Abort completely. Because we haven't written to the file yet,
            // the file on disk remains safe and uncorrupted.
            throw FileServiceException(FileErrorCode::CANNOT_OPERATE,
                                       "applyDiff failed at change block " + std::to_string(i + 1) +
                                       ": Search text not found in file " + path.string());
        }

        // Apply the replacement to our in-memory string
        current_contents.replace(pos, change.search_text.length(), change.replace_text);
    }

    // 4. Write the fully modified contents back to the file
    {
        std::ofstream outfile(path, std::ios::trunc);
        if (!outfile.is_open()) {
            throw FileServiceException(FileErrorCode::CANNOT_OPERATE,
                                       "applyDiff failed: Could not open file for writing at " + path.string());
        }
        outfile << current_contents;
    } // outfile closed
}

void FileService::editFile(const std::filesystem::path& path, const std::vector<Actions::TextEdit>& edits) {
    std::lock_guard<std::mutex> lock(m_mutex);

    // 1. Validate file exists
    if (!std::filesystem::exists(path)) {
        throw FileServiceException(FileErrorCode::NOT_FOUND,
                                   "editFile failed: File does not exist at " + path.string());
    }

    // 2. Read existing contents into memory
    std::string current_contents;
    {
        std::ifstream infile(path);
        if (!infile.is_open()) {
            throw FileServiceException(FileErrorCode::CANNOT_OPERATE,
                                       "editFile failed: Could not open file for reading at " + path.string());
        }
        std::ostringstream ss;
        ss << infile.rdbuf();
        current_contents = ss.str();
    } // infile closed

    // 3. Copy and Sort the edits in REVERSE order
    // This guarantees that earlier index offsets aren't ruined when text is injected or removed
    std::vector<Actions::TextEdit> sorted_edits = edits;
    std::sort(sorted_edits.begin(), sorted_edits.end(),
              [](const Actions::TextEdit& a, const Actions::TextEdit& b) {
                  return a.start_index > b.start_index;
              });

    // 4. Apply all edits sequentially
    for (size_t i = 0; i < sorted_edits.size(); ++i) {
        const auto& edit = sorted_edits[i];

        // Bounds validation
        if (edit.start_index > current_contents.length() ||
            edit.end_index > current_contents.length() ||
            edit.start_index > edit.end_index)
        {
            // Throw before writing anything if an agent hallucinates invalid indices
            throw FileServiceException(FileErrorCode::CANNOT_OPERATE,
                                       "editFile failed: Out of bounds indices (start: " + std::to_string(edit.start_index) +
                                       ", end: " + std::to_string(edit.end_index) + ") for file " + path.string());
        }

        // Apply replacement (calculate length to replace as end_index - start_index)
        size_t length_to_replace = edit.end_index - edit.start_index;
        current_contents.replace(edit.start_index, length_to_replace, edit.text);
    }

    // 5. Write the fully modified contents back to the file
    {
        std::ofstream outfile(path, std::ios::trunc);
        if (!outfile.is_open()) {
            throw FileServiceException(FileErrorCode::CANNOT_OPERATE,
                                       "editFile failed: Could not open file for writing at " + path.string());
        }
        outfile << current_contents;
    } // outfile closed
}