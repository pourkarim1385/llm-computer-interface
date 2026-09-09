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

    Q_INVOKABLE bool sendMessage(const QString &text) {
        const QString trimmed = text.trimmed();
        if (trimmed.isEmpty()) return false;



        if (m_bridge) {
            if (m_bridge->isWorking()) {
                return false;
            }
            qDebug() << "[InputBoxController] Dispatching to Bridge:" << trimmed;
            m_bridge->sendPrompt(trimmed);
            return true;
        }

        return false;
    }

private:
    AgentBridge *m_bridge = nullptr;
};