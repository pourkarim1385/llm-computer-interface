#include "AgentBridge.h"
#include "Repository/RepositoryManager.h"
#include <QMetaObject>
#include <QDebug>

AgentBridge::AgentBridge(std::shared_ptr<Orchestrator> orchestrator, QObject *parent)
    : QObject(parent), m_orchestrator(orchestrator) {
    if (m_orchestrator) {
        setupCallbacks();
    }
}

int AgentBridge::status() const {
    if (!m_orchestrator) return static_cast<int>(AgentStatus::Idle);
    return static_cast<int>(m_orchestrator->getStatus());
}

void AgentBridge::sendPrompt(const QString &prompt) {
    if (!m_orchestrator) return;
    std::string text = prompt.trimmed().toStdString();
    if (!text.empty()) {
        m_orchestrator->handleUserPrompt(text);
    }
}

void AgentBridge::createNewChat() {
    if (m_orchestrator) {
        qDebug() << "[AgentBridge] Requesting Orchestrator to create new chat...";
        m_orchestrator->createNewChat();
    }
}

void AgentBridge::setActiveChat(const QString &chatId) {
    if (!m_orchestrator) return;
    std::string id = chatId.toStdString();
    qDebug() << "[AgentBridge] Switching active chat to ID:" << chatId;
    m_orchestrator->setActiveChat(id);
}

void AgentBridge::stopExecution() {
    if (m_orchestrator) {
        m_orchestrator->requestStop();
    }
}

void AgentBridge::respondApproval(bool isApproved) {
    if (m_orchestrator) {
        m_orchestrator->handleUserApproval(isApproved);
    }
}

void AgentBridge::loadChatsFromRepository() {
    auto& chatRepo = agent::repository::RepositoryManager::getInstance().chat();

    std::vector<std::unique_ptr<agent::chat::ChatHistory>> histories = chatRepo.getAllHistories();

    QList<ChatItem> items;
    items.reserve(static_cast<qsizetype>(histories.size()));

    for (const auto& history : histories) {
        if (!history) continue;

        ChatItem item;
        item.id = QString::fromStdString(history->getId());
        std::string rawTitle = history->getTitle();
        item.title = rawTitle.empty() ? QStringLiteral("New Chat") : QString::fromStdString(rawTitle);
        item.isActive = false;

        items.append(item);
    }

    qDebug() << "[AgentBridge] Loaded" << items.size() << "chat sessions from database.";
    emit chatsLoaded(items);
}

void AgentBridge::setupCallbacks() {
    m_orchestrator->onMessageReceived = [this](const std::string& message, const Plan& /*plan*/) {
        QString qmsg = QString::fromStdString(message);
        QMetaObject::invokeMethod(this, [this, qmsg]() {
            emit messageReceived(qmsg);
        }, Qt::QueuedConnection);
    };

    m_orchestrator->onStatusChanged = [this](AgentStatus newStatus) {
        int statusVal = static_cast<int>(newStatus);
        QMetaObject::invokeMethod(this, [this, statusVal]() {
            emit statusChanged(statusVal);
        }, Qt::QueuedConnection);
    };

    m_orchestrator->onError = [this](const std::string& error) {
        QString qerr = QString::fromStdString(error);
        QMetaObject::invokeMethod(this, [this, qerr]() {
            emit errorOccurred(qerr);
        }, Qt::QueuedConnection);
    };

    m_orchestrator->onChatLoaded = [this](std::shared_ptr<agent::chat::ChatHistory> chat) {
        if (!chat) return;

        QString cId = QString::fromStdString(chat->getId());
        std::string rawTitle = chat->getTitle();
        QString cTitle = rawTitle.empty() ? QStringLiteral("New Chat") : QString::fromStdString(rawTitle);

        QMetaObject::invokeMethod(this, [this, cId, cTitle]() {
            emit chatSessionLoaded(cId, cTitle);
        }, Qt::QueuedConnection);
    };
}