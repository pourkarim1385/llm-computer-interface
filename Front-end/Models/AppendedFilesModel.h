#pragma once

#include <QAbstractListModel>
#include <QUrl>
#include <QFileInfo>
#include <vector>
#include "AppendedFileItem.h"

class AppendedFilesModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int fileCount READ fileCount NOTIFY fileCountChanged)
    Q_PROPERTY(qint64 totalFileSizeBytes READ totalFileSizeBytes NOTIFY totalFileSizeBytesChanged)

public:
    enum Roles {
        PathRole = Qt::UserRole + 1,
        FileNameRole,
        IsFolderRole,
        FileSizeRole,
        FormattedSizeRole,
        DescriptionRole,
        ExcludeNamesRole,
        ExcludeExtensionsRole
    };

    explicit AppendedFilesModel(QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] int count() const { return static_cast<int>(m_items.size()); }
    [[nodiscard]] int fileCount() const { return m_fileCount; }
    [[nodiscard]] qint64 totalFileSizeBytes() const { return m_totalFileSizeBytes; }

    const std::vector<AppendedFileItem>& items() const { return m_items; }

    Q_INVOKABLE bool addFile(const QUrl &fileUrl);
    Q_INVOKABLE bool addFolder(const QUrl &folderUrl);
    Q_INVOKABLE void removeAt(int index);
    Q_INVOKABLE void updateProperties(int index, const QString &description, const QString &excludeNames, const QString &excludeExtensions);
    Q_INVOKABLE void clear();
    Q_INVOKABLE QVariantMap get(int index) const;

signals:
    void countChanged();
    void fileCountChanged();
    void totalFileSizeBytesChanged();
    void errorOccurred(const QString &message);

private:
    static QString formatSize(qint64 bytes);

    std::vector<AppendedFileItem> m_items;
    int m_fileCount{0};
    qint64 m_totalFileSizeBytes{0};

    static constexpr int MAX_FILES = 10;
    static constexpr qint64 MAX_TOTAL_BYTES = 50 * 1024 * 1024; // 50 MB
};