#pragma once

#include <string>
#include <cstdint>

namespace agent::chat {

    class Message {
    public:
        Message() = default;

        Message(std::string id,
            std::string userInput,
            std::string llmResult = "",
            int64_t timestampUnixSec = 0);

        // Getters
        [[nodiscard]] const std::string& getId() const noexcept { return m_id; }
        [[nodiscard]] const std::string& getChatId() const noexcept { return m_chatId; }
        [[nodiscard]] const std::string& getUserInput() const noexcept { return m_userInput; }
        [[nodiscard]] const std::string& getLlmResult() const noexcept { return m_llmResult; }
        [[nodiscard]] int64_t getTimestampUnixSec() const noexcept { return m_timestampUnixSec; }

        // Setters 
        void setId(std::string id) { m_id = std::move(id); }
        void setChatId(std::string chatId) { m_chatId = std::move(chatId); }
        void setUserInput(std::string userInput) { m_userInput = std::move(userInput); }
        void setLlmResult(std::string llmResult) { m_llmResult = std::move(llmResult); }
        void setTimestampUnixSec(int64_t timestampUnixSec) noexcept { m_timestampUnixSec = timestampUnixSec; }

    private:
        std::string m_id;
        std::string m_chatId;
        std::string m_userInput;
        std::string m_llmResult;
        int64_t m_timestampUnixSec{ 0 };
    };

}

