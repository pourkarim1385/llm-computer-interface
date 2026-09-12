#include "Repository/SqlInterfaces/SettingsRepository.h"
#include "Repository/DatabaseManager.h"
#include <iostream>

namespace agent::repository {

    std::optional<agent::settings::UserSettings> SettingsRepository::getSettings() const {
        try {
            auto& db = DatabaseManager::getInstance().getDb();
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
            db.remove_all<agent::settings::UserSettings>();
            db.insert(settings);
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