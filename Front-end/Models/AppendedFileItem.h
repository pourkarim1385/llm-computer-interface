#pragma once

#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <string>
#include "Observation/Models/fileIncludeFilter.h"

struct AppendedFileItem {
    QString path;
    QString fileName;
    bool isFolder{false};
    qint64 fileSize{0};
    QString description;
    QString excludeNames;
    QString excludeExtensions;

    [[nodiscard]] fileIncludeFilter toFilter() const {
        fileIncludeFilter filter(description.toStdString());

        const QStringList names = excludeNames.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        for (const QString &name : names) {
            filter.addExcludeName(name.toStdString());
        }

        const QStringList extensions = excludeExtensions.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        for (const QString &ext : extensions) {
            QString cleanExt = ext.startsWith('.') ? ext.mid(1) : ext;
            filter.addExcludeExtension(cleanExt.toStdString());
        }

        return filter;
    }
};