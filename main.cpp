#include <iostream>
#include <QIcon>
#include "Core/Orchestrator.h"
#include "Repository/DatabaseManager.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include "Front-end/Controllers/InputBoxController.h"
#include "Front-end/Models/ChatListModel.h"
#include "Front-end/Controllers/NavigationController.h"
#include "Front-end/Models/InputBoxModel.h"
#include "Front-end/AgentBridge.h"
#include "Front-end/Models/AppendedFilesModel.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <memory>
#include "Core/Orchestrator.h"
#include "Repository/DatabaseManager.h"
#include "Front-end/AgentBridge.h"
#include "Front-end/Controllers/InputBoxController.h"
#include "Front-end/Models/ChatListModel.h"
#include "Front-end/Controllers/NavigationController.h"
#include "Front-end/Models/InputBoxModel.h"
#include "Front-end/Controllers/SettingsController.h"
#include "Repository/SqlInterfaces/SettingsRepository.h"

int main(int argc, char* argv[]) {
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");
    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/AccessibilityService/Front-end/assets/icon.svg"));
    const std::string apiKey = "Sample API key";
    const std::string endpoint = "https://SampleBaseUrl.com";
    agent::config::LLMProviderConfig myConfig = agent::config::LLMProviderConfig("SampleID", "SampleName", "SampleModelID", endpoint, apiKey, agent::config::ApiFormat::OpenAICompatible);
    WebSearch::SearchConfig mySConfig = WebSearch::SearchConfig{ "SampleTavilyAPIKey", 3 };
    agent::settings::UserSettings mySetting = agent::settings::UserSettings("aliAndReza", agent::repository::SettingsRepository::DEFAULT_SETTINGS_ID, "default settings");    mySetting.addProvider(myConfig);
    mySetting.setSearchProviderConfig(mySConfig);
    mySetting.setActiveProviderId("2");

    agent::repository::DatabaseManager::getInstance().initialize("agent_data.db");
    auto& repoManager = agent::repository::RepositoryManager::getInstance();
    if (!repoManager.settings().getSettings().has_value()) {
        repoManager.settings().saveSettings(mySetting);
    }

    auto orchestrator = std::make_shared<Orchestrator>();
    AgentBridge agentBridge(orchestrator);

    ChatListModel chatModel(&agentBridge);
    NavigationController navController(&agentBridge, &chatModel);
    AppendedFilesModel appendedFilesModel;
    InputBoxController inputController(&agentBridge, &appendedFilesModel);
    InputBoxModel inputModel;
    SettingsController settingsController(&agentBridge);


    QObject::connect(&agentBridge, &AgentBridge::chatSessionLoaded,
        &chatModel, &ChatListModel::addAndSelectChat);

    QObject::connect(&agentBridge, &AgentBridge::chatsLoaded,
        &chatModel, &ChatListModel::setChats);

    agentBridge.loadChatsFromRepository();

    if (chatModel.rowCount() == 0) {
        orchestrator->createNewChat();
    }
    else {
        chatModel.selectChat(0);
    }

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("agentBridge", &agentBridge);
    engine.rootContext()->setContextProperty("chatModel", &chatModel);
    engine.rootContext()->setContextProperty("navController", &navController);
    engine.rootContext()->setContextProperty("inputBoxController", &inputController);
    engine.rootContext()->setContextProperty("inputBoxModel", &inputModel);
    engine.rootContext()->setContextProperty("appendedFilesModel", &appendedFilesModel);
    engine.rootContext()->setContextProperty("settingsController", &settingsController);

    const QUrl url(QStringLiteral("qrc:/AccessibilityService/Front-end/Main.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject* obj, const QUrl& objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}