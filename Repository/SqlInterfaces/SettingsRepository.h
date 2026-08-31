#pragma once
#include "Config/UserSettings.h"
#include "../DatabaseManager.h"
#include <memory>
#include <string>

namespace agent::storage {
    class SettingsRepository {
    public:
        explicit SettingsRepository(std::shared_ptr<DatabaseManager> dbManager);

        // Loads basic info. Providers must be populated separately.
        settings::UserSettings loadSettings();
        bool saveSettings(const settings::UserSettings& settings);

    private:
        std::shared_ptr<DatabaseManager> m_dbManager;
        void setCustomValue(const std::string& key, const std::string& value);
        std::string getCustomValue(const std::string& key, const std::string& defaultValue = "");
    };
}