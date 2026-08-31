#pragma once
#include <externalLibs/sqlite/sqlite3.h>
#include <string>
#include <stdexcept>

namespace agent::storage {
    class DatabaseManager {
    public:
        explicit DatabaseManager(const std::string& dbPath);
        ~DatabaseManager();

        sqlite3* getConnection() const noexcept { return m_db; }
        void beginTransaction();
        void commit();
        void rollback();

    private:
        sqlite3* m_db{nullptr};
        void initializeSchema();
        void executeQuery(const std::string& query);
    };
}