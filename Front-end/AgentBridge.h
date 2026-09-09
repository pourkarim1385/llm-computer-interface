#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QVariantMap>
#include <memory>
#include "Core/Orchestrator.h"
#include "Front-end/Models/ChatItem.h"
#include "Front-end/Models/ChatFeedModel.h"

class AgentBridge : public QObject {
    Q_OBJECT
    Q_PROPERTY(int status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)
    Q_PROPERTY(bool isWorking READ isWorking NOTIFY statusChanged)
    Q_PROPERTY(ChatFeedModel* feedModel READ feedModel CONSTANT)

public:
    explicit AgentBridge(std::shared_ptr<Orchestrator> orchestrator, QObject *parent = nullptr);

    [[nodiscard]] int status() const;
    [[nodiscard]] QString statusText() const;
    [[nodiscard]] bool isWorking() const;
    ChatFeedModel* feedModel() { return &m_feedModel; }

    Q_INVOKABLE void sendPrompt(const QString &prompt);
    Q_INVOKABLE void createNewChat();
    Q_INVOKABLE void setActiveChat(const QString &chatId);
    Q_INVOKABLE void stopExecution();
    Q_INVOKABLE void respondApproval(bool isApproved);

    void loadChatsFromRepository();

    signals:
    void statusChanged(int newStatus);
    void messageReceived(const QString &message, const QVariantMap &plan);
    void errorOccurred(const QString &errorMsg);
    void taskCompleted();
    void chatSessionLoaded(const QString &id, const QString &title);
    void chatsLoaded(const QList<ChatItem> &chats);

private:
    void setupCallbacks();
    static QVariantMap serializePlan(const Plan &plan);

    std::shared_ptr<Orchestrator> m_orchestrator;
    ChatFeedModel m_feedModel;
    QString m_activeChatId;
};