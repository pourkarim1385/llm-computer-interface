#include "Repository/SqlInterfaces/ChatRepository.h"
#include "Repository/DatabaseManager.h"
#include <iostream>

namespace agent::repository {

    bool ChatRepository::saveHistory(const agent::chat::ChatHistory& history) {
        try {
            DatabaseManager::getInstance().getDb().replace(history);
            return true;
        }
        catch (const std::exception& e) {
            std::cerr << "[ChatRepo] Error saving history: " << e.what() << "\n";
            return false;
        }
    }

    std::unique_ptr<agent::chat::ChatHistory> ChatRepository::getHistory(const std::string& chatId) const {
        try {
            // get_pointer already returns a std::unique_ptr! We just pass it through.
            return DatabaseManager::getInstance().getDb().get_pointer<agent::chat::ChatHistory>(chatId);
        }
        catch (const std::exception& e) {
            std::cerr << "[ChatRepo] Error getting history: " << e.what() << "\n";
        }
        return nullptr;
    }

    std::vector<std::unique_ptr<agent::chat::ChatHistory>> ChatRepository::getAllHistories() const {
        std::vector<std::unique_ptr<agent::chat::ChatHistory>> histories;
        try {
            auto& db = DatabaseManager::getInstance().getDb();

            // To avoid copying the mutex, we select ONLY the IDs from the database...
            auto ids = db.select(&agent::chat::ChatHistory::getId);

            // ...and then load them as pointers!
            for (const auto& id : ids) {
                if (auto ptr = db.get_pointer<agent::chat::ChatHistory>(id)) {
                    histories.push_back(std::move(ptr));
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "[ChatRepo] Error getting all histories: " << e.what() << "\n";
        }
        return histories;
    }

    bool ChatRepository::saveMessage(const agent::chat::Message& msg) {
        try {
            DatabaseManager::getInstance().getDb().replace(msg);
            return true;
        }
        catch (const std::exception& e) {
            std::cerr << "[ChatRepo] Error saving message: " << e.what() << "\n";
            return false;
        }
    }

    std::vector<agent::chat::Message> ChatRepository::getMessagesForChat(const std::string& chatId) const {
        try {
            using namespace sqlite_orm;
            return DatabaseManager::getInstance().getDb().get_all<agent::chat::Message>(
                where(c(&agent::chat::Message::getChatId) == chatId)
            );
        }
        catch (...) { return {}; }
    }

} // namespace agent::repository