#include "Repository/SqlInterfaces/SettingsRepository.h"
#include "Repository/DatabaseManager.h"
#include <iostream>

namespace agent::repository {

    std::optional<agent::settings::UserSettings> SettingsRepository::getSettings() const {
        try {
            auto& db = DatabaseManager::getInstance().getDb();
            auto settings = db.get_pointer<agent::settings::UserSettings>(DEFAULT_SETTINGS_ID);
            if (settings) {
                return *settings;
            }

            auto allSettings = db.get_all<agent::settings::UserSettings>();
            if (!allSettings.empty()) {
                return allSettings.front();
            }
        }
        catch (const std::exception& e) {
            std::cerr << "[SettingsRepo] Error getting settings: " << e.what() << "\n";
        }
        return std::nullopt;
    }

    bool SettingsRepository::saveSettings(const agent::settings::UserSettings& settings) {
        try {
            auto& db = DatabaseManager::getInstance().getDb();
            agent::settings::UserSettings toSave = settings;
            if (toSave.email().empty()) {
                toSave.setEmail(DEFAULT_SETTINGS_ID);
            }
            db.replace(toSave);
            return true;
        }
        catch (const std::exception& e) {
            std::cerr << "[SettingsRepo] Error saving settings: " << e.what() << "\n";
            return false;
        }
    }

    bool SettingsRepository::deleteSettings() {
        try {
            auto& db = DatabaseManager::getInstance().getDb();
            db.remove_all<agent::settings::UserSettings>();
            return true;
        }
        catch (const std::exception& e) {
            std::cerr << "[SettingsRepo] Error deleting settings: " << e.what() << "\n";
            return false;
        }
    }

}