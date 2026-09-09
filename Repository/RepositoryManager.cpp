#include "Repository/RepositoryManager.h"

namespace agent::repository {

    RepositoryManager& RepositoryManager::getInstance() {
        static RepositoryManager instance;
        return instance;
    }

    RepositoryManager::RepositoryManager() {
        m_settingsRepo = std::make_unique<SettingsRepository>();
        m_chatRepo = std::make_unique<ChatRepository>();
    }

    SettingsRepository& RepositoryManager::settings() {
        return *m_settingsRepo;
    }

    ChatRepository& RepositoryManager::chat() {
        return *m_chatRepo;
    }

} // namespace agent::repository