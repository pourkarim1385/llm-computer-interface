#pragma once

#include "Repository/SqlInterfaces/SettingsRepository.h"
#include "Repository/SqlInterfaces/ChatRepository.h"
#include <memory>

namespace agent::repository {

    class RepositoryManager {
    public:
        // Singleton access
        static RepositoryManager& getInstance();

        // Prevent copying
        RepositoryManager(const RepositoryManager&) = delete;
        RepositoryManager& operator=(const RepositoryManager&) = delete;

        // Repository Accessors
        SettingsRepository& settings();
        ChatRepository& chat();

    private:
        RepositoryManager();
        ~RepositoryManager() = default;

        std::unique_ptr<SettingsRepository> m_settingsRepo;
        std::unique_ptr<ChatRepository> m_chatRepo;
    };

}