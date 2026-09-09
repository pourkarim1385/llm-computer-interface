#include "ChatFeedModel.h"

ChatFeedModel::ChatFeedModel(QObject *parent) : QAbstractListModel(parent) {}

int ChatFeedModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_turns.size();
}

QVariant ChatFeedModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_turns.size())
        return QVariant();

    const auto &turn = m_turns.at(index.row());
    switch (role) {
        case MessageIdRole: return turn.messageId;
        case UserPromptRole: return turn.userPrompt;
        case AssistantMarkdownRole: return turn.assistantMarkdown.isEmpty() ? QStringLiteral("") : turn.assistantMarkdown;
        case PlanDataRole: return turn.planData;
        case HasPlanRole: return turn.hasPlan;
        case IsPendingRole: return turn.isPending;
        default: return QVariant();
    }
}

QHash<int, QByteArray> ChatFeedModel::roleNames() const {
    return {
        {MessageIdRole, "messageId"},
        {UserPromptRole, "userPrompt"},
        {AssistantMarkdownRole, "assistantMarkdown"},
        {PlanDataRole, "planData"},
        {HasPlanRole, "hasPlan"},
        {IsPendingRole, "isPending"}
    };
}

void ChatFeedModel::setTurns(const QList<ChatTurn> &turns) {
    beginResetModel();
    m_turns = turns;
    endResetModel();
}

void ChatFeedModel::addUserPrompt(const QString &prompt) {
    beginInsertRows(QModelIndex(), m_turns.size(), m_turns.size());
    ChatTurn turn;
    turn.userPrompt = prompt;
    turn.assistantMarkdown = QStringLiteral("");
    turn.isPending = true;
    m_turns.append(turn);
    endInsertRows();
}

void ChatFeedModel::setAssistantResponse(const QString &markdown, const QVariantMap &plan) {
    if (m_turns.isEmpty()) return;

    int lastIdx = m_turns.size() - 1;
    m_turns[lastIdx].assistantMarkdown = markdown;
    m_turns[lastIdx].planData = plan;
    m_turns[lastIdx].hasPlan = !plan.isEmpty() && plan.contains("steps");
    m_turns[lastIdx].isPending = false;

    QModelIndex idx = createIndex(lastIdx, 0);
    emit dataChanged(idx, idx);
}

void ChatFeedModel::setLastTurnPending(bool pending) {
    if (m_turns.isEmpty()) return;
    int lastIdx = m_turns.size() - 1;
    m_turns[lastIdx].isPending = pending;
    QModelIndex idx = createIndex(lastIdx, 0);
    emit dataChanged(idx, idx, {IsPendingRole});
}

void ChatFeedModel::clear() {
    beginResetModel();
    m_turns.clear();
    endResetModel();
}