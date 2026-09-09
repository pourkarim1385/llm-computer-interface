#pragma once
#include <QAbstractListModel>
#include <QStringList>

class InputBoxModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles { TextRole = Qt::UserRole + 1 };

    explicit InputBoxModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        return m_items.size();
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() >= m_items.size())
            return QVariant();

        if (role == TextRole)
            return m_items.at(index.row());

        return QVariant();
    }

    QHash<int, QByteArray> roleNames() const override {
        return { {TextRole, "text"} };
    }

    Q_INVOKABLE void addItem(const QString &text) {
        beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
        m_items.append(text);
        endInsertRows();
    }

private:
    QStringList m_items;
};