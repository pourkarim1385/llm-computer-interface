#include "SettingsRepository.h"
#include "externalLibs/sqlite/sqlite3.h"

namespace agent::storage {
    SettingsRepository::SettingsRepository(std::shared_ptr<DatabaseManager> dbManager)
        : m_dbManager(std::move(dbManager)) {}

    void SettingsRepository::setCustomValue(const std::string& key, const std::string& value) {
        const char* sql = "INSERT INTO user_settings (key, value) VALUES (?, ?) ON CONFLICT(key) DO UPDATE SET value=excluded.value;";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(m_dbManager->getConnection(), sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, value.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    std::string SettingsRepository::getCustomValue(const std::string& key, const std::string& defaultValue) {
        const char* sql = "SELECT value FROM user_settings WHERE key = ?;";
        sqlite3_stmt* stmt;
        std::string result = defaultValue;
        if (sqlite3_prepare_v2(m_dbManager->getConnection(), sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            }
            sqlite3_finalize(stmt);
        }
        return result;
    }

    settings::UserSettings SettingsRepository::loadSettings() {
        settings::UserSettings s;
        s.setName(getCustomValue("name", ""));
        s.setEmail(getCustomValue("email", ""));
        s.setDescription(getCustomValue("description", ""));
        s.setStorageDirectory(getCustomValue("storageDirectory", "./agent_workspace"));
        s.setActiveProviderId(getCustomValue("activeProviderId", ""));
        return s;
    }

    bool SettingsRepository::saveSettings(const settings::UserSettings& settings) { //
        m_dbManager->beginTransaction();
        try {
            setCustomValue("name", settings.name());
            setCustomValue("email", settings.email());
            setCustomValue("description", settings.description());
            setCustomValue("storageDirectory", settings.storageDirectory());
            setCustomValue("activeProviderId", settings.activeProviderId());
            m_dbManager->commit();
            return true;
        } catch (...) {
            m_dbManager->rollback();
            return false;
        }
    }
}
