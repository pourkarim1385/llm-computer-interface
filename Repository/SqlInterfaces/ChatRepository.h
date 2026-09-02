#pragma once

#include "Context/ChatHistory.h"
#include "Context/Message.h"
#include <vector>
#include <memory>
#include <string>

namespace agent::repository {

    class ChatRepository {
    public:
        ChatRepository() = default;
        ~ChatRepository() = default;

        // 1. History Management
        bool saveHistory(const agent::chat::ChatHistory& history);

        // Changed from std::optional and std::vector<ChatHistory>
        std::unique_ptr<agent::chat::ChatHistory> getHistory(const std::string& chatId) const;
        std::vector<std::unique_ptr<agent::chat::ChatHistory>> getAllHistories() const;

        // 2. Message Management (Lazy Loading)
        bool saveMessage(const agent::chat::Message& msg);
        std::vector<agent::chat::Message> getMessagesForChat(const std::string& chatId) const;
    };

} // namespace agent::repository