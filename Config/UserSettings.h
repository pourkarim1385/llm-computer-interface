#pragma once

#include "LLM/LlmProviderConfig.h"
#include <string>
#include <vector>
#include <optional>

namespace agent::settings {

    class UserSettings {
    public:
        UserSettings();
        UserSettings(std::string name,
                     std::string email,
                     std::string description,
                     std::vector<config::LLMProviderConfig> providers = {},
                     std::string activeProviderId = "",
                     std::string storageDirectory = "./agent_workspace");

        // Identity Getters & Setters
        [[nodiscard]] const std::string& name() const noexcept { return m_name; }
        [[nodiscard]] const std::string& email() const noexcept { return m_email; }
        [[nodiscard]] const std::string& description() const noexcept { return m_description; }
        [[nodiscard]] const std::string& storageDirectory() const noexcept { return m_storageDirectory; }

        void setName(std::string name) { m_name = std::move(name); }
        void setEmail(std::string email) { m_email = std::move(email); }
        void setDescription(std::string description) { m_description = std::move(description); }
        void setStorageDirectory(std::string path) { m_storageDirectory = std::move(path); }

        // Provider Management
        [[nodiscard]] const std::vector<config::LLMProviderConfig>& providers() const noexcept { return m_providers; }
        [[nodiscard]] const std::string& activeProviderId() const noexcept { return m_activeProviderId; }

        void setActiveProviderId(std::string providerId);
        void addProvider(config::LLMProviderConfig provider);
        bool updateProvider(const config::LLMProviderConfig& provider);
        bool removeProvider(const std::string& providerId);

        [[nodiscard]] std::optional<config::LLMProviderConfig> getProvider(const std::string& providerId) const;
        [[nodiscard]] std::optional<config::LLMProviderConfig> getActiveProvider() const;

        [[nodiscard]] bool isValid() const noexcept;

    private:
        std::string m_name;
        std::string m_email;
        std::string m_description;
        std::string m_storageDirectory{"./agent_workspace"};

        std::vector<config::LLMProviderConfig> m_providers;
        std::string m_activeProviderId;
    };

} // namespace agent::settings