#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include "../AgentBridge.h"

class SettingsController : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool sendNotif READ sendNotif WRITE setSendNotif NOTIFY sendNotifChanged)
    Q_PROPERTY(QString aboutUrl READ aboutUrl CONSTANT)
    Q_PROPERTY(QString activeProviderId READ activeProviderId WRITE setActiveProviderId NOTIFY activeProviderIdChanged)
    Q_PROPERTY(QVariantList providers READ providers NOTIFY providersChanged)
    Q_PROPERTY(QString tavilyApiKey READ tavilyApiKey WRITE setTavilyApiKey NOTIFY tavilyApiKeyChanged)
    Q_PROPERTY(QString tavilyApiKeyMasked READ tavilyApiKeyMasked NOTIFY tavilyApiKeyChanged)
    Q_PROPERTY(qint64 tavilyCreditLimit READ tavilyCreditLimit WRITE setTavilyCreditLimit NOTIFY tavilyCreditLimitChanged)

public:
    explicit SettingsController(AgentBridge *bridge, QObject *parent = nullptr)
        : QObject(parent), m_bridge(bridge) {
        if (m_bridge) {
            connect(m_bridge, &AgentBridge::providersChanged, this, &SettingsController::providersChanged);
            connect(m_bridge, &AgentBridge::activeProviderChanged, this, &SettingsController::activeProviderIdChanged);
            connect(m_bridge, &AgentBridge::settingsUpdated, this, &SettingsController::syncFromBridge);
            syncFromBridge();
        }
    }

    [[nodiscard]] bool sendNotif() const noexcept { return m_sendNotif; }
    void setSendNotif(bool value) {
        if (m_sendNotif != value) {
            m_sendNotif = value;
            if (m_bridge) m_bridge->setSendNotif(value);
            emit sendNotifChanged();
        }
    }

    [[nodiscard]] QString aboutUrl() const noexcept {
        return QStringLiteral("https://github.com/pourkarim1385/llm-computer-interface");
    }

    [[nodiscard]] QString activeProviderId() const noexcept {
        return m_bridge ? m_bridge->activeProviderId() : QString();
    }
    void setActiveProviderId(const QString &providerId) {
        if (m_bridge) m_bridge->setActiveProvider(providerId);
    }

    [[nodiscard]] QVariantList providers() const {
        return m_bridge ? m_bridge->providers() : QVariantList();
    }

    [[nodiscard]] QString tavilyApiKey() const noexcept { return m_tavilyApiKey; }
    void setTavilyApiKey(const QString &key) {
        if (m_tavilyApiKey != key) {
            m_tavilyApiKey = key;
            emit tavilyApiKeyChanged();
        }
    }

    [[nodiscard]] QString tavilyApiKeyMasked() const {
        return m_bridge ? m_bridge->getTavilyApiKeyMasked() : QString();
    }

    [[nodiscard]] qint64 tavilyCreditLimit() const noexcept { return m_tavilyCreditLimit; }
    void setTavilyCreditLimit(qint64 limit) {
        if (m_tavilyCreditLimit != limit) {
            m_tavilyCreditLimit = limit;
            emit tavilyCreditLimitChanged();
        }
    }

    Q_INVOKABLE bool addProvider(const QString &name, const QString &baseUrl, const QString &apiKey, int formatIndex, const QString &modelId) {
        qDebug() << "[SettingsController] Adding provider:" << name;
        if (m_bridge) {
            bool ok = m_bridge->addProvider(name, baseUrl, apiKey, formatIndex, modelId);
            if (ok) {
                emit providersChanged();
                emit activeProviderIdChanged();
            }
            return ok;
        }
        return false;
    }

    Q_INVOKABLE bool removeProvider(const QString &providerId) {
        qDebug() << "[SettingsController] Removing provider:" << providerId;
        if (m_bridge) {
            bool ok = m_bridge->removeProvider(providerId);
            if (ok) {
                emit providersChanged();
                emit activeProviderIdChanged();
            }
            return ok;
        }
        return false;
    }

    Q_INVOKABLE QVariantMap getProviderDetails(const QString &providerId) const {
        return m_bridge ? m_bridge->getProviderDetails(providerId) : QVariantMap();
    }

    Q_INVOKABLE void resetUsage() {
        if (m_bridge) m_bridge->resetWebSearchUsage();
    }

    Q_INVOKABLE void resetToDefaults() {
        if (m_bridge) m_bridge->resetSettingsToDefaults();
    }

    Q_INVOKABLE bool clearStorage() {
        qDebug() << "[SettingsController] Requesting storage clear...";
        return m_bridge ? m_bridge->clearAllStorage() : false;
    }

    Q_INVOKABLE void saveSettings() {
        if (m_bridge) {
            m_bridge->updateWebSearchConfig(m_tavilyApiKey, m_tavilyCreditLimit);
            m_bridge->setSendNotif(m_sendNotif);
        }
    }

    Q_INVOKABLE void openAboutUrl() {
        QDesktopServices::openUrl(QUrl(aboutUrl()));
    }

signals:
    void sendNotifChanged();
    void activeProviderIdChanged();
    void providersChanged();
    void tavilyApiKeyChanged();
    void tavilyCreditLimitChanged();

private:
    void syncFromBridge() {
        if (!m_bridge) return;
        m_sendNotif = m_bridge->getSendNotif();
        m_tavilyApiKey = m_bridge->getTavilyApiKey();
        m_tavilyCreditLimit = m_bridge->getTavilyCreditLimit();
        emit sendNotifChanged();
        emit activeProviderIdChanged();
        emit tavilyApiKeyChanged();
        emit tavilyCreditLimitChanged();
        emit providersChanged();
    }

    AgentBridge *m_bridge = nullptr;
    bool m_sendNotif{true};
    QString m_tavilyApiKey;
    qint64 m_tavilyCreditLimit{1000};
};