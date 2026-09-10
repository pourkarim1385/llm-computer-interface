#pragma once
#include <QAbstractListModel>
#include <QList>
#include "ChatItem.h"

class ChatListModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum ChatRoles {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        IsActiveRole
    };

    explicit ChatListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void selectChat(int index);
    Q_INVOKABLE QString activeChatId() const;

public slots:
    void setChats(const QList<ChatItem> &chats);
    void addAndSelectChat(const QString &id, const QString &title);

    signals:
        void chatSelected(const QString &chatId);

private:
    QList<ChatItem> m_chats;
    int m_activeIndex = -1;
};