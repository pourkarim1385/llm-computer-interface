#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>
#include <string_view>

namespace agent::config {

    enum class ApiFormat {
        OpenAICompatible,
        Anthropic,
        Ollama,
        Gemini,
        Custom
    };

    class LLMProviderConfig {
    public:
        LLMProviderConfig();
        LLMProviderConfig(std::string id,
                          std::string name,
                          std::string model_id,
                          std::string base_url,
                          std::string api_key,
                          ApiFormat format,
                          uint32_t context_window = 128000,
                          uint32_t max_tokens = 4096,
                          double temperature = 0.1,
                          double top_p = 0.95,
                          uint32_t timeout_seconds = 60,
                          bool supports_vision = true,
                          bool supports_tool_calling = true,
                          std::unordered_map<std::string, std::string> custom_headers = {});

        // Getters
        [[nodiscard]] const std::string& id() const noexcept { return m_id; }
        [[nodiscard]] const std::string& name() const noexcept { return m_name; }
        [[nodiscard]] const std::string& model_id() const noexcept { return m_model_id; }
        [[nodiscard]] const std::string& base_url() const noexcept { return m_base_url; }
        [[nodiscard]] const std::string& api_key() const noexcept { return m_api_key; }
        [[nodiscard]] ApiFormat format() const noexcept { return m_format; }
        [[nodiscard]] uint32_t context_window() const noexcept { return m_context_window; }
        [[nodiscard]] uint32_t max_tokens() const noexcept { return m_max_tokens; }
        [[nodiscard]] double temperature() const noexcept { return m_temperature; }
        [[nodiscard]] double top_p() const noexcept { return m_top_p; }
        [[nodiscard]] uint32_t timeout_seconds() const noexcept { return m_timeout_seconds; }
        [[nodiscard]] bool supports_vision() const noexcept { return m_supports_vision; }
        [[nodiscard]] bool supports_tool_calling() const noexcept { return m_supports_tool_calling; }
        [[nodiscard]] const std::unordered_map<std::string, std::string>& custom_headers() const noexcept { return m_custom_headers; }

        // Setters
        void set_id(std::string id) { m_id = std::move(id); }
        void set_name(std::string name) { m_name = std::move(name); }
        void set_model_id(std::string model_id) { m_model_id = std::move(model_id); }
        void set_base_url(std::string base_url) { m_base_url = std::move(base_url); }
        void set_api_key(std::string api_key) { m_api_key = std::move(api_key); }
        void set_format(ApiFormat format) noexcept { m_format = format; }
        void set_context_window(uint32_t window) noexcept { m_context_window = window; }
        void set_max_tokens(uint32_t tokens) noexcept { m_max_tokens = tokens; }
        void set_temperature(double temp) noexcept { m_temperature = temp; }
        void set_top_p(double top_p) noexcept { m_top_p = top_p; }
        void set_timeout_seconds(uint32_t seconds) noexcept { m_timeout_seconds = seconds; }
        void set_supports_vision(bool supported) noexcept { m_supports_vision = supported; }
        void set_supports_tool_calling(bool supported) noexcept { m_supports_tool_calling = supported; }
        void set_custom_headers(std::unordered_map<std::string, std::string> headers) { m_custom_headers = std::move(headers); }

        // Helpers
        [[nodiscard]] bool is_valid() const noexcept;
        static std::string format_to_string(ApiFormat format);
        static ApiFormat string_to_format(std::string_view format_str);

    private:
        std::string m_id;
        std::string m_name;
        std::string m_model_id;
        std::string m_base_url;
        std::string m_api_key;
        ApiFormat m_format{ApiFormat::OpenAICompatible};
        uint32_t m_context_window{128000};
        uint32_t m_max_tokens{4096};
        double m_temperature{0.1};
        double m_top_p{0.95};
        uint32_t m_timeout_seconds{60};
        bool m_supports_vision{true};
        bool m_supports_tool_calling{true};
        std::unordered_map<std::string, std::string> m_custom_headers;
    };

} // namespace agent::config