#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QPointer>
#include <QDebug>
#include "../AgentBridge.h"
#include "../Models/AppendedFilesModel.h"

class InputBoxController : public QObject {
    Q_OBJECT

public:
    explicit InputBoxController(AgentBridge* bridge, AppendedFilesModel* filesModel = nullptr, QObject* parent = nullptr)
        : QObject(parent), m_bridge(bridge), m_filesModel(filesModel) {
    }

    void setFilesModel(AppendedFilesModel* filesModel) {
        m_filesModel = filesModel;
    }

    Q_INVOKABLE bool sendMessage(const QString& text, const QVariantMap& observationFlags = QVariantMap()) {
        const QString trimmed = text.trimmed();
        const bool hasFiles = (m_filesModel && m_filesModel->count() > 0);

        if (trimmed.isEmpty() && !hasFiles) {
            return false;
        }

        if (!m_bridge || m_bridge->isWorking()) {
            return false;
        }

        if (hasFiles) {
            const auto items = m_filesModel->items();
            for (const auto& item : items) {
                qDebug() << "[InputBoxController] Requesting file analysis for:" << item.path;
                m_bridge->fileAnalyzeRequest(item.path.toStdString(), item.toFilter());
            }
            m_filesModel->clear();
        }

        qDebug() << "[InputBoxController] Dispatching to Bridge:" << trimmed << "with flags:" << observationFlags;
        m_bridge->sendPrompt(trimmed, observationFlags);
        return true;
    }

    Q_INVOKABLE void stopExecution() {
        if (m_bridge) {
            qDebug() << "[InputBoxController] Dispatching stop to Bridge";
            m_bridge->stopExecution();
        }
    }

private:
    QPointer<AgentBridge> m_bridge;
    QPointer<AppendedFilesModel> m_filesModel;
};