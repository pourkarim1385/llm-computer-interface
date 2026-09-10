#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <memory>
#include "Core/Orchestrator.h"
#include "Front-end/Models/ChatItem.h"

class AgentBridge : public QObject {
    Q_OBJECT
    Q_PROPERTY(int status READ status NOTIFY statusChanged)

public:
    explicit AgentBridge(std::shared_ptr<Orchestrator> orchestrator, QObject *parent = nullptr);

    [[nodiscard]] int status() const;

    Q_INVOKABLE void sendPrompt(const QString &prompt);
    Q_INVOKABLE void createNewChat();
    Q_INVOKABLE void setActiveChat(const QString &chatId);
    Q_INVOKABLE void stopExecution();
    Q_INVOKABLE void respondApproval(bool isApproved);

    void loadChatsFromRepository();

    signals:
        void statusChanged(int newStatus);
    void messageReceived(const QString &message);
    void errorOccurred(const QString &errorMsg);
    void taskCompleted();

    void chatSessionLoaded(const QString &id, const QString &title);
    void chatsLoaded(const QList<ChatItem> &chats);

private:
    void setupCallbacks();

    std::shared_ptr<Orchestrator> m_orchestrator;
};