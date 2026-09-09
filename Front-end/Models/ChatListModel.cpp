#include "ChatListModel.h"
#include "Front-end/AgentBridge.h"
#include <QDebug>

ChatListModel::ChatListModel(AgentBridge *bridge, QObject *parent)
    : QAbstractListModel(parent), m_bridge(bridge) {
}

int ChatListModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_chats.count();
}

QVariant ChatListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_chats.size() || index.row() < 0)
        return QVariant();

    const auto &item = m_chats.at(index.row());
    switch (role) {
    case IdRole: return item.id;
    case TitleRole: return item.title;
    case IsActiveRole: return item.isActive;
    default: return QVariant();
    }
}

QHash<int, QByteArray> ChatListModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "chatId";
    roles[TitleRole] = "chatTitle";
    roles[IsActiveRole] = "isActive";
    return roles;
}

void ChatListModel::setChats(const QList<ChatItem> &chats) {
    beginResetModel();
    m_chats = chats;
    m_activeIndex = -1;
    for (int i = 0; i < m_chats.size(); ++i) {
        if (m_chats[i].isActive) {
            m_activeIndex = i;
            break;
        }
    }
    endResetModel();
}

void ChatListModel::addAndSelectChat(const QString &id, const QString &title) {
    for (int i = 0; i < m_chats.size(); ++i) {
        if (m_chats[i].id == id) {
            selectChat(i);
            return;
        }
    }

    if (m_activeIndex >= 0 && m_activeIndex < m_chats.size()) {
        m_chats[m_activeIndex].isActive = false;
        QModelIndex prev = createIndex(m_activeIndex, 0);
        emit dataChanged(prev, prev, {IsActiveRole});
    }

    beginInsertRows(QModelIndex(), 0, 0);
    m_chats.prepend({id, title, true});
    m_activeIndex = 0;
    endInsertRows();
}

void ChatListModel::selectChat(int index) {
    if (index < 0 || index >= m_chats.size())
        return;

    if (m_activeIndex == index)
        return;

    if (m_bridge && m_bridge->isWorking()) {
        qDebug() << "[ChatListModel] Agent is busy. Switch rejected.";
        return;
    }

    if (m_bridge) {
        const bool success = m_bridge->setActiveChat(m_chats[index].id);
        if (!success) return;
    }

    if (m_activeIndex >= 0 && m_activeIndex < m_chats.size()) {
        m_chats[m_activeIndex].isActive = false;
        QModelIndex prev = createIndex(m_activeIndex, 0);
        emit dataChanged(prev, prev, {IsActiveRole});
    }

    m_activeIndex = index;
    m_chats[m_activeIndex].isActive = true;
    QModelIndex cur = createIndex(m_activeIndex, 0);
    emit dataChanged(cur, cur, {IsActiveRole});

    emit chatSelected(m_chats[m_activeIndex].id);
}

QString ChatListModel::activeChatId() const {
    if (m_activeIndex >= 0 && m_activeIndex < m_chats.size())
        return m_chats[m_activeIndex].id;
    return QString();
}

bool ChatListModel::deleteChat(const QString &chatId) {
    if (m_bridge) {
        if (!m_bridge->deleteChat(chatId)) {
            return false;
        }
    }
    return removeRowById(chatId);
}

bool ChatListModel::renameChat(const QString &chatId, const QString &newTitle) {
    QString trimmed = newTitle.trimmed();
    if (trimmed.isEmpty()) return false;

    if (m_bridge) {
        if (!m_bridge->renameChat(chatId, trimmed)) {
            return false;
        }
    }
    updateTitleInMemory(chatId, trimmed);
    return true;
}

bool ChatListModel::removeRowById(const QString &chatId) {
    int targetIndex = -1;
    for (int i = 0; i < m_chats.size(); ++i) {
        if (m_chats[i].id == chatId) {
            targetIndex = i;
            break;
        }
    }
    if (targetIndex < 0) return false;

    beginRemoveRows(QModelIndex(), targetIndex, targetIndex);
    bool wasActive = m_chats[targetIndex].isActive;
    m_chats.removeAt(targetIndex);

    if (wasActive) {
        m_activeIndex = -1;
    } else if (m_activeIndex > targetIndex) {
        m_activeIndex--;
    }
    endRemoveRows();
    return true;
}

void ChatListModel::updateTitleInMemory(const QString &chatId, const QString &newTitle) {
    for (int i = 0; i < m_chats.size(); ++i) {
        if (m_chats[i].id == chatId) {
            m_chats[i].title = newTitle;
            QModelIndex idx = createIndex(i, 0);
            emit dataChanged(idx, idx, {TitleRole});
            break;
        }
    }
}