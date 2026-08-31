#pragma once

#include "Context/ChatHistory.h"
#include "Context/Message.h"
#include "Repository/DatabaseManager.h"
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace agent::storage {

    struct ChatSummary {
        std::string id;
        std::string title;
        int64_t lastUpdatedUnixSec{0};
        size_t messageCount{0};
    };

    class ChatRepository {
    public:
        explicit ChatRepository(std::shared_ptr<DatabaseManager> dbManager);
        ~ChatRepository() = default;

        // Retrieves a lightweight list of all chats (metadata only)
        std::vector<ChatSummary> getChatSummaries();

        // Fully reconstructs a ChatHistory including its Plan and all Messages
        std::optional<chat::ChatHistory> loadChat(const std::string& chatId);

        // Saves or updates the chat metadata, plan, and all messages within it
        bool saveChat(const chat::ChatHistory& chat);

        // Efficiently appends a single message without re-saving the entire chat history
        bool appendMessage(const std::string& chatId, const chat::Message& msg);

        // Updates just the Plan JSON for a specific chat
        bool updatePlan(const std::string& chatId, const chat::Plan& plan);

        // Deletes a chat (Cascades to messages automatically via SQLite Foreign Keys)
        bool deleteChat(const std::string& chatId);

    private:
        std::shared_ptr<DatabaseManager> m_dbManager;
        
        // Helper to get current unix time
        static int64_t getCurrentTimeSec();
    };

}