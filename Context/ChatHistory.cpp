#include "ChatHistory.h"
#include <sstream>

namespace agent::chat {

    ChatHistory::ChatHistory(std::string _chatId,
                             std::string _title,
                             config::LLMProviderConfig _usedConfig)
            : chatId(std::move(_chatId))
            , title(std::move(_title))
            , usedConfig(std::move(_usedConfig))
    {
    }

    void ChatHistory::addMessage(Message msg) {
        messages.push_back(std::move(msg));
    }

    void ChatHistory::clearMessages() noexcept {
        messages.clear();
    }

    Message* ChatHistory::getLastMessage() {
        if (messages.empty()) {
            return nullptr;
        }
        return &messages.back();
    }

    const Message* ChatHistory::getLastMessage() const {
        if (messages.empty()) {
            return nullptr;
        }
        return &messages.back();
    }

    bool ChatHistory::updateLastMessage(std::string userInput, std::string llmResult) {
        if (messages.empty()) {
            return false;
        }
        messages.back().setUserInput(std::move(userInput));
        messages.back().setLlmResult(std::move(llmResult));
        return true;
    }

    bool ChatHistory::updateLastMessageResult(std::string llmResult) {
        if (messages.empty()) {
            return false;
        }
        messages.back().setLlmResult(std::move(llmResult));
        return true;
    }

    std::string ChatHistory::getLastMessagesContext(const size_t count) const {
        if (messages.empty()) {
            return "";
        }

        size_t startIndex = 0;
        if (count > 0 && count < messages.size()) {
            startIndex = messages.size() - count;
        }

        std::ostringstream oss;
        for (size_t i = startIndex; i < messages.size(); ++i) {
            oss << "[User Prompt]: " << messages[i].getUserInput() << "\n"
                << "[LLM Result]: " << messages[i].getLlmResult();

            if (i + 1 < messages.size()) {
                oss << "\n\n";
            }
        }

        return oss.str();
    }

}