#pragma once

#include "Message.h"
#include "Actuation/ExecutionCallStack.h"
#include "Config/LLM/LlmProviderConfig.h"
#include <vector>
#include <string>

namespace agent::chat {
    struct Step {
        std::string title;
        std::string content;
        bool isDone{false};
    };

    struct Plan {
        std::string name;
        std::string description;
        std::vector<Step> steps;
    };

    class ChatHistory {
    public:
        ChatHistory() = default;
        explicit ChatHistory(std::string _chatId,
                             std::string _title = "New Chat",
                             config::LLMProviderConfig _usedConfig = {});

        // Getters
        [[nodiscard]] const std::string& getId() const noexcept { return chatId; }
        [[nodiscard]] const std::string& getTitle() const noexcept { return title; }
        [[nodiscard]] const config::LLMProviderConfig& getUsedConfig() const noexcept { return usedConfig; }
        [[nodiscard]] const ExecutionCallStack& getExecutionCallStack() const noexcept { return stack; }
        [[nodiscard]] ExecutionCallStack& getMutableExecutionCallStack() noexcept { return stack; }
        [[nodiscard]] const std::vector<Message>& getMessages() const noexcept { return messages; }
        [[nodiscard]] std::vector<Message>& getMutableMessages() noexcept { return messages; }
        [[nodiscard]] const Plan& getPlan() const noexcept { return plan; }
        [[nodiscard]] Plan& getMutablePlan() noexcept { return plan; }

        // Setters (Chat ID has NO setter - Immutable)
        void setTitle(std::string newTitle) { newTitle = std::move(newTitle); }
        void setUsedConfig(config::LLMProviderConfig config) { usedConfig = std::move(config); }
        void updatePlan(const Plan& newPlane) {plan = newPlane;}
        void setId(std::string id) { chatId = std::move(id); }

        // Message Collection Management
        void addMessage(Message msg);
        void clearMessages() noexcept;
        [[nodiscard]] bool isEmpty() const noexcept { return messages.empty(); }
        [[nodiscard]] size_t messageCount() const noexcept { return messages.size(); }

        // Last Message Inspection & Editing
        [[nodiscard]] Message* getLastMessage();
        [[nodiscard]] const Message* getLastMessage() const;
        bool updateLastMessage(std::string userInput, std::string llmResult);
        bool updateLastMessageResult(std::string llmResult);

        // Context Window Extraction
        [[nodiscard]] std::string getLastMessagesContext(size_t count = 0) const;
        [[nodiscard]] std::string getAllMessagesContext() const { return getLastMessagesContext(0); }

    private:
        std::string chatId;
        std::string title;
        config::LLMProviderConfig usedConfig;
        ExecutionCallStack stack;
        std::vector<Message> messages;
        Plan plan;
    };

}