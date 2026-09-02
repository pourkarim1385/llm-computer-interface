#pragma once

#include <string>
#include <memory>
#include <sqlite_orm/sqlite_orm.h>
#include "Config/UserSettings.h"
#include "Context/ChatHistory.h"
#include "Context/Message.h"
#include "Repository/DbMappers.h"

namespace agent::repository {

    // 1. Define the schema tables
    inline auto initStorage(const std::string& dbPath) {
        using namespace sqlite_orm;
        return make_storage(dbPath,
            // 1. Updated Messages Table
            make_table("messages",
                make_column("id", &agent::chat::Message::getId, &agent::chat::Message::setId, primary_key()),
                make_column("chat_id", &agent::chat::Message::getChatId, &agent::chat::Message::setChatId),
                make_column("user_input", &agent::chat::Message::getUserInput, &agent::chat::Message::setUserInput),
                make_column("llm_result", &agent::chat::Message::getLlmResult, &agent::chat::Message::setLlmResult),
                make_column("timestamp", &agent::chat::Message::getTimestampUnixSec, &agent::chat::Message::setTimestampUnixSec)
            ),
            // 2. New Chat History Table
            make_table("chat_history",
                make_column("id", &agent::chat::ChatHistory::getId, &agent::chat::ChatHistory::setId, primary_key()),
                make_column("title", &agent::chat::ChatHistory::getTitle, &agent::chat::ChatHistory::setTitle),
                make_column("plan", &agent::chat::ChatHistory::getPlan, &agent::chat::ChatHistory::updatePlan)
                // Note: We use the JSON mapper we wrote earlier for 'plan'!
            ),
            // 3. User Settings Table (Unchanged)
            make_table("user_settings",
                make_column("name", &agent::settings::UserSettings::name, &agent::settings::UserSettings::setName),
                make_column("email", &agent::settings::UserSettings::email, &agent::settings::UserSettings::setEmail),
                make_column("description", &agent::settings::UserSettings::description, &agent::settings::UserSettings::setDescription),
                make_column("active_provider_id", &agent::settings::UserSettings::activeProviderId, &agent::settings::UserSettings::setActiveProviderId),
                make_column("providers", &agent::settings::UserSettings::providers, &agent::settings::UserSettings::setProviders)
            )
        );
    }

    // 2. Create an alias for the complex database type
    using Storage = decltype(initStorage(""));

    // 3. The Manager class to handle the database lifecycle
    class DatabaseManager {
    public:
        static DatabaseManager& getInstance();

        void initialize(const std::string& dbPath);
        Storage& getDb();

    private:
        DatabaseManager() = default; // Private constructor for Singleton
        std::unique_ptr<Storage> m_db;
    };
}