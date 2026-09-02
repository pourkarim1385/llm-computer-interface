#pragma once

#include "Config/UserSettings.h"
#include <optional>
#include <string>

namespace agent::repository {

    class SettingsRepository {
    public:
        // We only expect one UserSettings row for a local application.
        // We can enforce a constant ID (e.g., "default") for it.
        static constexpr const char* DEFAULT_SETTINGS_ID = "default_user";

        SettingsRepository() = default;
        ~SettingsRepository() = default;

        // Retrieves the user settings. Returns std::nullopt if none exist yet.
        [[nodiscard]] std::optional<agent::settings::UserSettings> getSettings() const;

        // Saves or updates the user settings
        bool saveSettings(const agent::settings::UserSettings& settings);

        // Deletes the settings (useful for a "Factory Reset" feature)
        bool deleteSettings();
    };

} // namespace agent::repository