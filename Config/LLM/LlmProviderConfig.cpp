#include "LlmProviderConfig.h"
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>

namespace agent::config {

    namespace {
// Fallback standard C++ UUID v4 generator
        std::string generate_uuid_v4() {
            static thread_local std::random_device rd;
            static thread_local std::mt19937_64 gen(rd());
            static thread_local std::uniform_int_distribution<uint64_t> dis;

            uint64_t ab = dis(gen);
            uint64_t cd = dis(gen);

            ab = (ab & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
            cd = (cd & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

            std::stringstream ss;
            ss << std::hex << std::setfill('0')
               << std::setw(8)  << (ab >> 32) << "-"
               << std::setw(4)  << ((ab >> 16) & 0xFFFF) << "-"
               << std::setw(4)  << (ab & 0xFFFF) << "-"
               << std::setw(4)  << (cd >> 48) << "-"
               << std::setw(12) << (cd & 0xFFFFFFFFFFFFULL);

            return ss.str();
        }
    } // namespace

    LLMProviderConfig::LLMProviderConfig()
            : m_id(generate_uuid_v4())
    {
    }

    LLMProviderConfig::LLMProviderConfig(std::string id,
                                         std::string name,
                                         std::string model_id,
                                         std::string base_url,
                                         std::string api_key,
                                         ApiFormat format,
                                         uint32_t context_window,
                                         uint32_t max_tokens,
                                         double temperature,
                                         double top_p,
                                         uint32_t timeout_seconds,
                                         bool supports_vision,
                                         bool supports_tool_calling,
                                         std::unordered_map<std::string, std::string> custom_headers)
            : m_id(id.empty() ? generate_uuid_v4() : std::move(id))
            , m_name(std::move(name))
            , m_model_id(std::move(model_id))
            , m_base_url(std::move(base_url))
            , m_api_key(std::move(api_key))
            , m_format(format)
            , m_context_window(context_window)
            , m_max_tokens(max_tokens)
            , m_temperature(temperature)
            , m_top_p(top_p)
            , m_timeout_seconds(timeout_seconds)
            , m_supports_vision(supports_vision)
            , m_supports_tool_calling(supports_tool_calling)
            , m_custom_headers(std::move(custom_headers))
    {
    }

    bool LLMProviderConfig::is_valid() const noexcept {
        return !m_name.empty() &&
               !m_model_id.empty() &&
               !m_base_url.empty();
    }

    std::string LLMProviderConfig::format_to_string(ApiFormat format) {
        switch (format) {
            case ApiFormat::OpenAICompatible: return "openai";
            case ApiFormat::Anthropic:        return "anthropic";
            case ApiFormat::Ollama:           return "ollama";
            case ApiFormat::Gemini:           return "gemini";
            case ApiFormat::Custom:           return "custom";
        }
        return "openai";
    }

    ApiFormat LLMProviderConfig::string_to_format(std::string_view format_str) {
        std::string lower(format_str);
        std::transform(lower.begin(), lower.end(), lower.begin(),
                       [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

        if (lower == "anthropic") return ApiFormat::Anthropic;
        if (lower == "ollama")    return ApiFormat::Ollama;
        if (lower == "gemini")    return ApiFormat::Gemini;
        if (lower == "custom")    return ApiFormat::Custom;
        return ApiFormat::OpenAICompatible;
    }

} // namespace agent::config