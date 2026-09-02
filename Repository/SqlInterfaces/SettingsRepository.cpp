#include "Repository/SqlInterfaces/SettingsRepository.h"
#include "Repository/DatabaseManager.h"
#include <iostream>

namespace agent::repository {

    std::optional<agent::settings::UserSettings> SettingsRepository::getSettings() const {
        try {
            auto& db = DatabaseManager::getInstance().getDb();

            // For a single-user local app, we usually just grab the first row, 
            // or fetch by our specific default ID. We'll use the default ID here.
            // (Note: We use a string "default_user" for the email/name as the primary key if needed,
            // but assuming your schema uses email as the implicit unique key, we can just get all and return the first).

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
            db.replace(settings);
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

} // namespace agent::repository