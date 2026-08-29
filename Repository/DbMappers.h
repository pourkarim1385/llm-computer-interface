#pragma once

#include <nlohmann/json.hpp>
#include "../Context/ChatHistory.h"
#include "../Config/LLM/LlmProviderConfig.h"
#include <string>

// Define serialization as free functions in the same namespace as the target types (ADL requirement)
namespace agent::chat {
    using json = nlohmann::json;

    void to_json(json& j, const Step& step);
    void from_json(const json& j, Step& step);

    void to_json(json& j, const Plan& plan);
    void from_json(const json& j, Plan& plan);
}

namespace agent::storage {
    class Mapper {
    public:
        // Helpers remain as static class methods
        static std::string serializePlan(const chat::Plan& plan);
        static chat::Plan deserializePlan(const std::string& serializedPlan);
        static std::string serializeHeaders(const std::unordered_map<std::string, std::string>& headers);
        static std::unordered_map<std::string, std::string> deserializeHeaders(const std::string& json_str);
    };
}