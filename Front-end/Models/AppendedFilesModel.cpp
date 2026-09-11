#include "AppendedFilesModel.h"
#include <QFileInfo>
#include <QDebug>

AppendedFilesModel::AppendedFilesModel(QObject *parent)
    : QAbstractListModel(parent) {}

int AppendedFilesModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_items.size());
}

QVariant AppendedFilesModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_items.size())) {
        return {};
    }

    const auto &item = m_items[index.row()];
    switch (role) {
        case PathRole: return item.path;
        case FileNameRole: return item.fileName;
        case IsFolderRole: return item.isFolder;
        case FileSizeRole: return item.fileSize;
        case FormattedSizeRole: return formatSize(item.fileSize);
        case DescriptionRole: return item.description;
        case ExcludeNamesRole: return item.excludeNames;
        case ExcludeExtensionsRole: return item.excludeExtensions;
        default: return {};
    }
}

QHash<int, QByteArray> AppendedFilesModel::roleNames() const {
    return {
        {PathRole, "filePath"},
        {FileNameRole, "fileName"},
        {IsFolderRole, "isFolder"},
        {FileSizeRole, "fileSize"},
        {FormattedSizeRole, "formattedSize"},
        {DescriptionRole, "description"},
        {ExcludeNamesRole, "excludeNames"},
        {ExcludeExtensionsRole, "excludeExtensions"}
    };
}

bool AppendedFilesModel::addFile(const QUrl &fileUrl) {
    QString localPath = fileUrl.isLocalFile() ? fileUrl.toLocalFile() : fileUrl.path();
    if (localPath.isEmpty()) {
        localPath = fileUrl.toString();
    }

#ifdef Q_OS_WIN
    if (localPath.startsWith('/') && localPath.length() > 2 && localPath[2] == ':') {
        localPath.remove(0, 1);
    }
#endif

    qDebug() << "[AppendedFilesModel] Adding file from URL:" << fileUrl << "-> Local path:" << localPath;

    QFileInfo fileInfo(localPath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        qWarning() << "[AppendedFilesModel] File validation failed! Path:" << localPath;
        emit errorOccurred(QStringLiteral("File does not exist or is invalid: ") + localPath);
        return false;
    }

    if (m_fileCount >= MAX_FILES) {
        emit errorOccurred(QStringLiteral("Cannot attach more than 10 files."));
        return false;
    }

    qint64 size = fileInfo.size();
    if (m_totalFileSizeBytes + size > MAX_TOTAL_BYTES) {
        emit errorOccurred(QStringLiteral("Total attached files exceed 50 MB limit."));
        return false;
    }

    for (const auto &it : m_items) {
        if (it.path == localPath) {
            emit errorOccurred(QStringLiteral("File is already attached: ") + fileInfo.fileName());
            return false;
        }
    }

    beginInsertRows(QModelIndex(), static_cast<int>(m_items.size()), static_cast<int>(m_items.size()));
    AppendedFileItem item;
    item.path = localPath;
    item.fileName = fileInfo.fileName();
    item.isFolder = false;
    item.fileSize = size;
    m_items.push_back(item);
    m_fileCount++;
    m_totalFileSizeBytes += size;
    endInsertRows();

    qDebug() << "[AppendedFilesModel] File added successfully. Count:" << m_items.size();

    emit countChanged();
    emit fileCountChanged();
    emit totalFileSizeBytesChanged();
    return true;
}

bool AppendedFilesModel::addFolder(const QUrl &folderUrl) {
    QString localPath = folderUrl.isLocalFile() ? folderUrl.toLocalFile() : folderUrl.path();
    if (localPath.isEmpty()) {
        localPath = folderUrl.toString();
    }

#ifdef Q_OS_WIN
    if (localPath.startsWith('/') && localPath.length() > 2 && localPath[2] == ':') {
        localPath.remove(0, 1);
    }
#endif

    qDebug() << "[AppendedFilesModel] Adding folder from URL:" << folderUrl << "-> Local path:" << localPath;

    QFileInfo folderInfo(localPath);
    if (!folderInfo.exists() || !folderInfo.isDir()) {
        qWarning() << "[AppendedFilesModel] Folder validation failed! Path:" << localPath;
        emit errorOccurred(QStringLiteral("Folder does not exist or is invalid: ") + localPath);
        return false;
    }

    for (const auto &it : m_items) {
        if (it.path == localPath) {
            emit errorOccurred(QStringLiteral("Folder is already attached: ") + folderInfo.fileName());
            return false;
        }
    }

    beginInsertRows(QModelIndex(), static_cast<int>(m_items.size()), static_cast<int>(m_items.size()));
    AppendedFileItem item;
    item.path = localPath;
    item.fileName = folderInfo.fileName();
    item.isFolder = true;
    item.fileSize = 0;
    m_items.push_back(item);
    endInsertRows();

    qDebug() << "[AppendedFilesModel] Folder added successfully. Count:" << m_items.size();

    emit countChanged();
    return true;
}

void AppendedFilesModel::removeAt(int index) {
    if (index < 0 || index >= static_cast<int>(m_items.size())) return;

    beginRemoveRows(QModelIndex(), index, index);
    const auto &item = m_items[index];
    if (!item.isFolder) {
        m_fileCount--;
        m_totalFileSizeBytes -= item.fileSize;
        emit fileCountChanged();
        emit totalFileSizeBytesChanged();
    }
    m_items.erase(m_items.begin() + index);
    endRemoveRows();

    emit countChanged();
}

void AppendedFilesModel::updateProperties(int index, const QString &description, const QString &excludeNames, const QString &excludeExtensions) {
    if (index < 0 || index >= static_cast<int>(m_items.size())) return;

    m_items[index].description = description;
    m_items[index].excludeNames = excludeNames;
    m_items[index].excludeExtensions = excludeExtensions;

    QModelIndex modelIdx = createIndex(index, 0);
    emit dataChanged(modelIdx, modelIdx, {DescriptionRole, ExcludeNamesRole, ExcludeExtensionsRole});
}

void AppendedFilesModel::clear() {
    if (m_items.empty()) return;

    beginResetModel();
    m_items.clear();
    m_fileCount = 0;
    m_totalFileSizeBytes = 0;
    endResetModel();

    emit countChanged();
    emit fileCountChanged();
    emit totalFileSizeBytesChanged();
}

QVariantMap AppendedFilesModel::get(int index) const {
    if (index < 0 || index >= static_cast<int>(m_items.size())) return {};

    const auto &it = m_items[index];
    QVariantMap map;
    map[QStringLiteral("filePath")] = it.path;
    map[QStringLiteral("fileName")] = it.fileName;
    map[QStringLiteral("isFolder")] = it.isFolder;
    map[QStringLiteral("description")] = it.description;
    map[QStringLiteral("excludeNames")] = it.excludeNames;
    map[QStringLiteral("excludeExtensions")] = it.excludeExtensions;
    return map;
}

QString AppendedFilesModel::formatSize(qint64 bytes) {
    if (bytes < 1024) return QString::number(bytes) + " B";
    double kb = bytes / 1024.0;
    if (kb < 1024.0) return QString::number(kb, 'f', 1) + " KB";
    double mb = kb / 1024.0;
    return QString::number(mb, 'f', 1) + " MB";
}