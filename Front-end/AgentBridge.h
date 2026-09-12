#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QVariantMap>
#include <QVariantList>
#include <memory>
#include "Core/Orchestrator.h"
#include "Front-end/Models/ChatItem.h"
#include "Front-end/Models/ChatFeedModel.h"
#include "Observation/Models/fileIncludeFilter.h"
#include "Observation/Services/WorldStateBuilderService.h"

class AgentBridge : public QObject {
    Q_OBJECT
    Q_PROPERTY(int status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)
    Q_PROPERTY(bool isWorking READ isWorking NOTIFY statusChanged)
    Q_PROPERTY(ChatFeedModel* feedModel READ feedModel CONSTANT)
    Q_PROPERTY(QString activeProviderName READ activeProviderName NOTIFY activeProviderChanged)
    Q_PROPERTY(QVariantList providers READ providers NOTIFY providersChanged)

public:
    explicit AgentBridge(std::shared_ptr<Orchestrator> orchestrator, QObject *parent = nullptr);

    [[nodiscard]] int status() const;
    [[nodiscard]] QString statusText() const;
    [[nodiscard]] bool isWorking() const;
    ChatFeedModel* feedModel() { return &m_feedModel; }

    [[nodiscard]] QString activeProviderName() const;
    [[nodiscard]] QVariantList providers() const;
    Q_INVOKABLE void setActiveProvider(const QString &providerId);

    Q_INVOKABLE void sendPrompt(const QString &prompt, const QVariantMap &observationFlags = QVariantMap());
    Q_INVOKABLE void createNewChat();
    Q_INVOKABLE bool setActiveChat(const QString &chatId);
    Q_INVOKABLE void stopExecution();
    Q_INVOKABLE void respondApproval(bool isApproved);
    Q_INVOKABLE bool deleteChat(const QString &chatId);
    Q_INVOKABLE bool renameChat(const QString &chatId, const QString &newTitle);
    bool fileAnalyzeRequest(const std::string& targetPath, const fileIncludeFilter targetFilter = fileIncludeFilter());

    void loadChatsFromRepository();

signals:
    void statusChanged(int newStatus);
    void messageReceived(const QString &message, const QVariantMap &plan);
    void errorOccurred(const QString &errorMsg);
    void taskCompleted();
    void chatSessionLoaded(const QString &id, const QString &title);
    void chatsLoaded(const QList<ChatItem> &chats);
    void activeProviderChanged();
    void providersChanged();

private:
    void setupCallbacks();
    static QVariantMap serializePlan(const Plan &plan);

    std::shared_ptr<Orchestrator> m_orchestrator;
    ChatFeedModel m_feedModel;
    QString m_activeChatId;
};