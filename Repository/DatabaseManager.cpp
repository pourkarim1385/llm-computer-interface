#include "DatabaseManager.h"

namespace agent::storage {
    DatabaseManager::DatabaseManager(const std::string& dbPath) {
        if (sqlite3_open(dbPath.c_str(), &m_db) != SQLITE_OK) {
            throw std::runtime_error("Failed to open SQLite DB: " + std::string(sqlite3_errmsg(m_db)));
        }
        executeQuery("PRAGMA foreign_keys = ON;");
        initializeSchema();
    }

    DatabaseManager::~DatabaseManager() {
        if (m_db) sqlite3_close(m_db);
    }

    void DatabaseManager::executeQuery(const std::string& query) {
        char* errMsg = nullptr;
        if (sqlite3_exec(m_db, query.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            std::string err = errMsg;
            sqlite3_free(errMsg);
            throw std::runtime_error("SQL Error: " + err);
        }
    }

    void DatabaseManager::beginTransaction() { executeQuery("BEGIN TRANSACTION;"); }
    void DatabaseManager::commit() { executeQuery("COMMIT;"); }
    void DatabaseManager::rollback() { executeQuery("ROLLBACK;"); }

    void DatabaseManager::initializeSchema() {
        const std::string schema = R"(
            CREATE TABLE IF NOT EXISTS user_settings (
                key TEXT PRIMARY KEY,
                value TEXT NOT NULL
            );
            CREATE TABLE IF NOT EXISTS llm_providers (
                id TEXT PRIMARY KEY,
                name TEXT NOT NULL,
                model_id TEXT NOT NULL,
                base_url TEXT NOT NULL,
                api_key_encrypted BLOB NOT NULL,
                api_format INTEGER NOT NULL,
                context_window INTEGER,
                max_tokens INTEGER,
                temperature REAL,
                top_p REAL,
                timeout_seconds INTEGER,
                supports_vision INTEGER,
                supports_tool_calling INTEGER,
                custom_headers_json TEXT
            );
            CREATE TABLE IF NOT EXISTS chats (
                id TEXT PRIMARY KEY,
                title TEXT NOT NULL,
                used_config_id TEXT,
                plan_json TEXT,
                created_at INTEGER NOT NULL,
                updated_at INTEGER NOT NULL,
                FOREIGN KEY(used_config_id) REFERENCES llm_providers(id) ON DELETE SET NULL
            );
            CREATE TABLE IF NOT EXISTS messages (
                id TEXT PRIMARY KEY,
                chat_id TEXT NOT NULL,
                user_input TEXT NOT NULL,
                llm_result TEXT,
                timestamp_sec INTEGER NOT NULL,
                FOREIGN KEY(chat_id) REFERENCES chats(id) ON DELETE CASCADE
            );
        )";
        executeQuery(schema);
    }
}