#include "userSettings.h"
#include <algorithm>

namespace agent::settings {

    UserSettings::UserSettings()
            : m_storageDirectory("./agent_workspace")
    {
    }

    UserSettings::UserSettings(std::string name,
                               std::string email,
                               std::string description,
                               std::vector<config::LLMProviderConfig> providers,
                               std::string activeProviderId,
                               std::string storageDirectory)
            : m_name(std::move(name))
            , m_email(std::move(email))
            , m_description(std::move(description))
            , m_storageDirectory(std::move(storageDirectory))
            , m_providers(std::move(providers))
            , m_activeProviderId(std::move(activeProviderId))
    {
        if (m_activeProviderId.empty() && !m_providers.empty()) {
            m_activeProviderId = m_providers.front().id();
        }
    }

    void UserSettings::setActiveProviderId(std::string providerId) {
        m_activeProviderId = std::move(providerId);
    }

    void UserSettings::addProvider(config::LLMProviderConfig provider) {
        if (m_activeProviderId.empty()) {
            m_activeProviderId = provider.id();
        }
        m_providers.push_back(std::move(provider));
    }

    bool UserSettings::updateProvider(const config::LLMProviderConfig& provider) {
        auto it = std::find_if(m_providers.begin(), m_providers.end(),
                               [&](const config::LLMProviderConfig& p) {
                                   return p.id() == provider.id();
                               });

        if (it != m_providers.end()) {
            *it = provider;
            return true;
        }
        return false;
    }

    bool UserSettings::removeProvider(const std::string& providerId) {
        auto it = std::find_if(m_providers.begin(), m_providers.end(),
                               [&](const config::LLMProviderConfig& p) {
                                   return p.id() == providerId;
                               });

        if (it == m_providers.end()) {
            return false;
        }

        m_providers.erase(it);

        if (m_activeProviderId == providerId) {
            m_activeProviderId = m_providers.empty() ? "" : m_providers.front().id();
        }

        return true;
    }

    std::optional<config::LLMProviderConfig> UserSettings::getProvider(const std::string& providerId) const {
        auto it = std::find_if(m_providers.begin(), m_providers.end(),
                               [&](const config::LLMProviderConfig& p) {
                                   return p.id() == providerId;
                               });

        if (it != m_providers.end()) {
            return *it;
        }
        return std::nullopt;
    }

    std::optional<config::LLMProviderConfig> UserSettings::getActiveProvider() const {
        if (m_activeProviderId.empty()) {
            if (!m_providers.empty()) {
                return m_providers.front();
            }
            return std::nullopt;
        }
        return getProvider(m_activeProviderId);
    }

    bool UserSettings::isValid() const noexcept {
        return !m_name.empty() && !m_email.empty();
    }

} // namespace agent::settings