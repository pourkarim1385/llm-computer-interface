#pragma once
#include <QObject>
#include <QDebug>
#include "../AgentBridge.h"

class NavigationController : public QObject {
    Q_OBJECT

public:
    explicit NavigationController(AgentBridge *bridge, QObject *parent = nullptr)
        : QObject(parent), m_bridge(bridge) {}

    Q_INVOKABLE void createNewChat() {
        qDebug() << "[NavigationController] Creating new chat session...";
        if (m_bridge) {
            m_bridge->createNewChat();
        }
    }

    Q_INVOKABLE void navigateBack() {
        qDebug() << "[NavigationController] Navigating backward...";
    }

    Q_INVOKABLE void navigateForward() {
        qDebug() << "[NavigationController] Navigating forward...";
    }

private:
    AgentBridge *m_bridge = nullptr;
};