#pragma once
#include <QObject>
#include <QString>
#include <QDebug>
#include "../AgentBridge.h"

class InputBoxController : public QObject {
    Q_OBJECT

public:
    explicit InputBoxController(AgentBridge *bridge, QObject *parent = nullptr)
        : QObject(parent), m_bridge(bridge) {}

    Q_INVOKABLE void sendMessage(const QString &text) {
        const QString trimmed = text.trimmed();
        if (trimmed.isEmpty()) return;

        qDebug() << "[InputBoxController] Dispatching to Bridge:" << trimmed;

        if (m_bridge) {
            m_bridge->sendPrompt(trimmed);
        }
    }

private:
    AgentBridge *m_bridge = nullptr;
};