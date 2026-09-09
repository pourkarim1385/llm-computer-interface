#pragma once
#include <QAbstractListModel>
#include <QList>
#include "ChatItem.h"

class AgentBridge;

class ChatListModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum ChatRoles {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        IsActiveRole
    };

    explicit ChatListModel(AgentBridge *bridge, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void selectChat(int index);
    Q_INVOKABLE QString activeChatId() const;

    Q_INVOKABLE bool deleteChat(const QString &chatId);
    Q_INVOKABLE bool renameChat(const QString &chatId, const QString &newTitle);

public slots:
    void setChats(const QList<ChatItem> &chats);
    void addAndSelectChat(const QString &id, const QString &title);

    signals:
        void chatSelected(const QString &chatId);

private:
    bool removeRowById(const QString &chatId);
    void updateTitleInMemory(const QString &chatId, const QString &newTitle);

    AgentBridge *m_bridge = nullptr;
    QList<ChatItem> m_chats;
    int m_activeIndex = -1;
};