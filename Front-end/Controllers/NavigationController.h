#pragma once
#include <QObject>
#include <QPointer>
#include <QVector>
#include <QDebug>
#include "../AgentBridge.h"
#include "../Models/ChatListModel.h"

class NavigationController : public QObject {
    Q_OBJECT

public:
    explicit NavigationController(AgentBridge* bridge, ChatListModel* chatModel, QObject* parent = nullptr)
        : QObject(parent), m_bridge(bridge), m_chatModel(chatModel) {

        if (m_chatModel) {
            connect(m_chatModel, &ChatListModel::chatSelected, this, &NavigationController::recordVisit);
        }
    }

    Q_INVOKABLE void createNewChat() {
        if (m_bridge) m_bridge->createNewChat();
    }

    Q_INVOKABLE void navigateBack() {
        if (m_historyIndex > 0) {
            m_historyIndex--;
            loadChatFromHistory();
        }
    }

    Q_INVOKABLE void navigateForward() {
        if (m_historyIndex < m_history.size() - 1) {
            m_historyIndex++;
            loadChatFromHistory();
        }
    }

private:
    void recordVisit(const QString& chatId) {
        if (m_isNavigating) return;

        if (m_historyIndex < m_history.size() - 1) {
            m_history.resize(m_historyIndex + 1);
        }

        if (m_history.isEmpty() || m_history.last() != chatId) {
            m_history.append(chatId);
            m_historyIndex = m_history.size() - 1;
        }
    }

    void loadChatFromHistory() {
        if (m_chatModel && m_historyIndex >= 0 && m_historyIndex < m_history.size()) {
            m_isNavigating = true; 

            QString targetId = m_history[m_historyIndex];

            for (int i = 0; i < m_chatModel->rowCount(); ++i) {
                if (m_chatModel->data(m_chatModel->index(i, 0), ChatListModel::IdRole).toString() == targetId) {
                    m_chatModel->selectChat(i);
                    break;
                }
            }

            m_isNavigating = false;
        }
    }

    QPointer<AgentBridge> m_bridge;
    QPointer<ChatListModel> m_chatModel;

    QVector<QString> m_history;
    int m_historyIndex = -1;
    bool m_isNavigating = false;
};