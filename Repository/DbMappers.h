#pragma once

// 1. Standard Library Includes
#include <vector>
#include <string>
#include <unordered_map>

// 2. Third-Party Includes
#include <nlohmann/json.hpp>
#include <sqlite_orm/sqlite_orm.h>

// 3. Project Includes (ORDER MATTERS!)
#include "Config/LLM/LlmProviderConfig.h" 
#include "Context/ChatHistory.h"
#include "Security/SecretVault.h"

// Set up the JSON alias for convenience
using json = nlohmann::json;

namespace agent::chat {
    // 1. JSON for Step
    inline void to_json(json& j, const Step& s) {
        j = json{ {"title", s.title}, {"content", s.content}, {"isDone", s.isDone} };
    }
    inline void from_json(const json& j, Step& s) {
        j.at("title").get_to(s.title);
        j.at("content").get_to(s.content);
        j.at("isDone").get_to(s.isDone);
    }

    // 2. JSON for Plan
    inline void to_json(json& j, const Plan& p) {
        j = json{ {"name", p.name}, {"description", p.description}, {"steps", p.steps} };
    }
    inline void from_json(const json& j, Plan& p) {
        j.at("name").get_to(p.name);
        j.at("description").get_to(p.description);
        j.at("steps").get_to(p.steps);
    }
}

namespace agent::config {
    // 1. Serialize C++ object to JSON
    inline void to_json(json& j, const LLMProviderConfig& c) {
        j = json{
            {"id", c.id()},
            {"name", c.name()},
            {"model_id", c.model_id()},
            {"base_url", c.base_url()},
            {"api_key", agent::security::SecretVault::encrypt(c.api_key())},
            {"format", static_cast<int>(c.format())},
            {"context_window", c.context_window()},
            {"max_tokens", c.max_tokens()},
            {"temperature", c.temperature()},
            {"top_p", c.top_p()},
            {"timeout_seconds", c.timeout_seconds()},
            {"supports_vision", c.supports_vision()},
            {"supports_tool_calling", c.supports_tool_calling()},
            {"custom_headers", c.custom_headers()}
        };
    }

    // 2. Deserialize JSON back to C++ object
    inline void from_json(const json& j, LLMProviderConfig& c) {
        c.set_id(j.value("id", ""));
        c.set_name(j.value("name", ""));
        c.set_model_id(j.value("model_id", ""));
        c.set_base_url(j.value("base_url", ""));
        c.set_api_key(agent::security::SecretVault::decrypt(j.value("api_key", "")));
        c.set_format(static_cast<ApiFormat>(j.value("format", 0)));
        c.set_context_window(j.value("context_window", 128000));
        c.set_max_tokens(j.value("max_tokens", 4096));
        c.set_temperature(j.value("temperature", 0.1));
        c.set_top_p(j.value("top_p", 0.95));
        c.set_timeout_seconds(j.value("timeout_seconds", 60));
        c.set_supports_vision(j.value("supports_vision", true));
        c.set_supports_tool_calling(j.value("supports_tool_calling", true));
        c.set_custom_headers(j.value("custom_headers", std::unordered_map<std::string, std::string>{}));
    }
}

namespace sqlite_orm {
    // ==========================================
    // MAPPERS FOR: std::vector<LLMProviderConfig>
    // ==========================================
    template<> struct type_printer<std::vector<agent::config::LLMProviderConfig>> : public text_printer {};

    template<> struct statement_binder<std::vector<agent::config::LLMProviderConfig>> {
        int bind(sqlite3_stmt* stmt, int index, const std::vector<agent::config::LLMProviderConfig>& value) const {
            return statement_binder<std::string>().bind(stmt, index, nlohmann::json(value).dump());
        }
    };

    template<> struct field_printer<std::vector<agent::config::LLMProviderConfig>> {
        std::string operator()(const std::vector<agent::config::LLMProviderConfig>& t) const {
            return nlohmann::json(t).dump();
        }
    };

    template<> struct row_extractor<std::vector<agent::config::LLMProviderConfig>> {
        std::vector<agent::config::LLMProviderConfig> extract(const char* row_value) const {
            if (row_value) return nlohmann::json::parse(row_value).get<std::vector<agent::config::LLMProviderConfig>>();
            return {};
        }
        std::vector<agent::config::LLMProviderConfig> extract(sqlite3_stmt* stmt, int columnIndex) const {
            auto str = row_extractor<std::string>().extract(stmt, columnIndex);
            if (!str.empty()) return nlohmann::json::parse(str).get<std::vector<agent::config::LLMProviderConfig>>();
            return {};
        }
    };

    // ==========================================
    // MAPPERS FOR: agent::chat::Plan
    // ==========================================
    template<> struct type_printer<agent::chat::Plan> : public text_printer {};

    template<> struct statement_binder<agent::chat::Plan> {
        int bind(sqlite3_stmt* stmt, int index, const agent::chat::Plan& value) const {
            return statement_binder<std::string>().bind(stmt, index, nlohmann::json(value).dump());
        }
    };

    template<> struct field_printer<agent::chat::Plan> {
        std::string operator()(const agent::chat::Plan& t) const {
            return nlohmann::json(t).dump();
        }
    };

    template<> struct row_extractor<agent::chat::Plan> {
        agent::chat::Plan extract(const char* row_value) const {
            if (row_value) return nlohmann::json::parse(row_value).get<agent::chat::Plan>();
            return {};
        }
        agent::chat::Plan extract(sqlite3_stmt* stmt, int columnIndex) const {
            auto str = row_extractor<std::string>().extract(stmt, columnIndex);
            if (!str.empty()) return nlohmann::json::parse(str).get<agent::chat::Plan>();
            return {};
        }
    };
}