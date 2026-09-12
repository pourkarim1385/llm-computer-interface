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

QString AgentBridge::statusText() const {
    if (!m_orchestrator) return QString();
    switch (m_orchestrator->getStatus()) {
        case AgentStatus::Observing: return QStringLiteral("Observing");
        case AgentStatus::Thinking: return QStringLiteral("Thinking");
        case AgentStatus::WaitingForApproval: return QStringLiteral("Waiting for approval");
        case AgentStatus::Executing: return QStringLiteral("Executing");
        case AgentStatus::Error: return QStringLiteral("Error");
        case AgentStatus::Idle:
        default: return QString();
    }
}

bool AgentBridge::isWorking() const {
    if (!m_orchestrator) return false;
    return m_orchestrator->getStatus() != AgentStatus::Idle;
}

QString AgentBridge::activeProviderName() const {
    auto& settingsRepo = agent::repository::RepositoryManager::getInstance().settings();
    auto settingsOpt = settingsRepo.getSettings();
    if (!settingsOpt) return QString();

    const auto& userSettings = *settingsOpt;
    auto activeProv = userSettings.getActiveProvider();
    if (activeProv.has_value()) {
        return QString("%1:%2").arg(
            QString::fromStdString(activeProv->name()),
            QString::fromStdString(activeProv->model_id())
        );
    }
    return QString();
}

QVariantList AgentBridge::providers() const {
    QVariantList list;
    auto& settingsRepo = agent::repository::RepositoryManager::getInstance().settings();
    auto settingsOpt = settingsRepo.getSettings();
    if (!settingsOpt) return list;

    const auto& userSettings = *settingsOpt;
    const std::string activeId = userSettings.activeProviderId();

    for (const auto& prov : userSettings.providers()) {
        QVariantMap item;
        item["id"] = QString::fromStdString(prov.id());
        item["name"] = QString::fromStdString(prov.name());
        item["modelId"] = QString::fromStdString(prov.model_id());
        item["displayName"] = QString("%1:%2").arg(
            QString::fromStdString(prov.name()),
            QString::fromStdString(prov.model_id())
        );
        item["isActive"] = (prov.id() == activeId);
        list.append(item);
    }
    return list;
}

void AgentBridge::setActiveProvider(const QString &providerId) {
    auto& settingsRepo = agent::repository::RepositoryManager::getInstance().settings();
    auto settingsOpt = settingsRepo.getSettings();
    if (!settingsOpt) return;

    auto userSettings = *settingsOpt;
    userSettings.setActiveProviderId(providerId.toStdString());

    if (settingsRepo.saveSettings(userSettings)) {
        emit activeProviderChanged();
        emit providersChanged();
    }
}

void AgentBridge::sendPrompt(const QString &prompt, const QVariantMap &observationFlags) {
    if (!m_orchestrator) return;
    QString text = prompt.trimmed();
    if (text.isEmpty()) return;

    m_feedModel.addUserPrompt(text);

    ObservationFlags flags;
    if (!observationFlags.isEmpty()) {
        if (observationFlags.contains("captureVision"))
            flags.captureVision = observationFlags.value("captureVision").toBool();
        if (observationFlags.contains("captureFullAccessibility"))
            flags.captureFullAccessibility = observationFlags.value("captureFullAccessibility").toBool();
        if (observationFlags.contains("captureActiveWindowAccessibility"))
            flags.captureActiveWindowAccessibility = observationFlags.value("captureActiveWindowAccessibility").toBool();
        if (observationFlags.contains("captureTargetWindowAccessibility"))
            flags.captureTargetWindowAccessibility = observationFlags.value("captureTargetWindowAccessibility").toBool();
        if (observationFlags.contains("targetWindow"))
            flags.targetWindow = observationFlags.value("targetWindow").toString().toStdString();
        if (observationFlags.contains("captureClipboard"))
            flags.captureClipboard = observationFlags.value("captureClipboard").toBool();
        if (observationFlags.contains("captureDesktop"))
            flags.captureDesktop = observationFlags.value("captureDesktop").toBool();
        if (observationFlags.contains("captureNewScreenMetrics"))
            flags.captureNewScreenMetrics = observationFlags.value("captureNewScreenMetrics").toBool();
    }

    m_orchestrator->handleUserPrompt(text.toStdString(), flags);
}

void AgentBridge::createNewChat() {
    if (m_orchestrator) {
        m_feedModel.clear();
        m_orchestrator->createNewChat();
    }
}

bool AgentBridge::setActiveChat(const QString &chatId) {
    if (!m_orchestrator) return false;

    std::string id = chatId.toStdString();

    if (!m_orchestrator->setActiveChat(id)) {
        return false;
    }

    m_activeChatId = chatId;

    auto& chatRepo = agent::repository::RepositoryManager::getInstance().chat();
    std::vector<agent::chat::Message> rawMessages = chatRepo.getMessagesForChat(id);
    auto historyPtr = chatRepo.getHistory(id);

    QList<ChatTurn> turns;
    turns.reserve(static_cast<qsizetype>(rawMessages.size()));

    for (size_t i = 0; i < rawMessages.size(); ++i) {
        const auto& msg = rawMessages[i];
        ChatTurn turn;
        turn.messageId = QString::fromStdString(msg.getId());
        turn.userPrompt = QString::fromStdString(msg.getUserInput());
        turn.assistantMarkdown = QString::fromStdString(msg.getResult());
        turn.isPending = false;

        if (historyPtr && i == rawMessages.size() - 1 && !historyPtr->getPlan().steps.empty()) {
            turn.planData = serializePlan(historyPtr->getPlan());
            turn.hasPlan = true;
        }

        turns.append(turn);
    }

    m_feedModel.setTurns(turns);
    return true;
}

void AgentBridge::stopExecution() {
    if (m_orchestrator) m_orchestrator->requestStop();
}

void AgentBridge::respondApproval(bool isApproved) {
    if (m_orchestrator) m_orchestrator->handleUserApproval(isApproved);
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
        item.isActive = (item.id == m_activeChatId);
        items.append(item);
    }
    emit chatsLoaded(items);
}

QVariantMap AgentBridge::serializePlan(const Plan &plan) {
    QVariantMap map;
    map["name"] = QString::fromStdString(plan.name);
    map["description"] = QString::fromStdString(plan.description);

    QVariantList stepsList;
    for (const auto &step : plan.steps) {
        QVariantMap stepMap;
        stepMap["title"] = QString::fromStdString(step.title);
        stepMap["content"] = QString::fromStdString(step.content);
        stepMap["isDone"] = step.isDone;
        stepsList.append(stepMap);
    }
    map["steps"] = stepsList;
    return map;
}

void AgentBridge::setupCallbacks() {
    m_orchestrator->onMessageReceived = [this](const std::string& message, const Plan& plan) {
        const QString qmsg = QString::fromStdString(message);
        QVariantMap planMap = serializePlan(plan);

        QMetaObject::invokeMethod(this, [this, qmsg, planMap]() {
            m_feedModel.setAssistantResponse(qmsg, planMap);
            emit messageReceived(qmsg, planMap);
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
            m_feedModel.setLastTurnPending(false);
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

bool AgentBridge::deleteChat(const QString &chatId) {
    if (!m_orchestrator) return false;

    if (m_orchestrator->getStatus() != AgentStatus::Idle) {
        qWarning() << "[AgentBridge] Cannot delete chat while orchestrator is busy!";
        return false;
    }

    std::string id = chatId.toStdString();
    auto& chatRepo = agent::repository::RepositoryManager::getInstance().chat();
    if (!chatRepo.deleteChat(id)) {
        return false;
    }

    if (chatId == m_activeChatId) {
        createNewChat();
    }

    return true;
}

bool AgentBridge::renameChat(const QString &chatId, const QString &newTitle) {
    QString trimmed = newTitle.trimmed();
    if (trimmed.isEmpty()) return false;

    auto& chatRepo = agent::repository::RepositoryManager::getInstance().chat();
    return chatRepo.updateChatTitle(chatId.toStdString(), trimmed.toStdString());
}

bool AgentBridge::fileAnalyzeRequest(const std::string& targetPath, const fileIncludeFilter targetFilter) {
    return WorldStateBuilderService::getInstance().fileAnalyzeRequest(targetPath, targetFilter);
}