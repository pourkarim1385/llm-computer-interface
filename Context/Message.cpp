#include "Message.h"

namespace agent::chat {

    Message::Message(std::string id,
                     std::string userInput,
                     std::string llmResult,
                     int64_t timestampUnixSec)
            : m_id(std::move(id))
            , m_userInput(std::move(userInput))
            , m_llmResult(std::move(llmResult))
            , m_timestampUnixSec(timestampUnixSec)
    {
    }

}