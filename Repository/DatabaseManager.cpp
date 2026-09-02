#include "Repository/DatabaseManager.h"
#include <stdexcept>

namespace agent::repository {

    DatabaseManager& DatabaseManager::getInstance() {
        static DatabaseManager instance;
        return instance;
    }

    void DatabaseManager::initialize(const std::string& dbPath) {
        // Create the database connection
        m_db = std::make_unique<Storage>(initStorage(dbPath));

        // This generates the SQL CREATE TABLE commands if they don't exist
        m_db->sync_schema();
    }

    Storage& DatabaseManager::getDb() {
        if (!m_db) {
            throw std::runtime_error("Critical Error: DatabaseManager::initialize() must be called before getDb()");
        }
        return *m_db;
    }
}