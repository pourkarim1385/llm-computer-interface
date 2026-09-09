#pragma once

#include <QAbstractListModel>
#include <QVariantMap>
#include <QList>

struct ChatTurn {
    QString messageId;
    QString userPrompt;
    QString assistantMarkdown;
    QVariantMap planData;
    bool hasPlan{false};
    bool isPending{false};
};

class ChatFeedModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        MessageIdRole = Qt::UserRole + 1,
        UserPromptRole,
        AssistantMarkdownRole,
        PlanDataRole,
        HasPlanRole,
        IsPendingRole
    };

    explicit ChatFeedModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setTurns(const QList<ChatTurn> &turns);
    void addUserPrompt(const QString &prompt);
    void setAssistantResponse(const QString &markdown, const QVariantMap &plan);
    void setLastTurnPending(bool pending);
    void clear();

private:
    QList<ChatTurn> m_turns;
};