#include <iostream>

#include "Core/Orchestrator.h"
#include "Repository/DatabaseManager.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "Front-end/Controllers/InputBoxController.h"
#include "Front-end/Models/ChatListModel.h"
#include "Front-end/Controllers/NavigationController.h"
#include "Front-end/Models/InputBoxModel.h"
#include "Front-end/AgentBridge.h"

using namespace std;
/*
int main(int argc, const char * argv[]) {
    cout << "[System] Initializing database...\n";
    string apiKey = "sk-OzgzQqIc8azSnEH9Lzn5EYx1mLabqH2tizw99nVWGdTD0KE3";
    string endpoint = "https://api.gapgpt.app/v1/chat/completions";
    agent::config::LLMProviderConfig myConfig = agent::config::LLMProviderConfig("1", "gapgpt", "nigger", endpoint, apiKey, agent::config::ApiFormat::OpenAICompatible);
    agent::settings::UserSettings mySetting = agent::settings::UserSettings("aliAndReza", "random", "i am not epstien");
    mySetting.addProvider(myConfig);
    mySetting.setActiveProviderId("1");

    agent::repository::DatabaseManager::getInstance().initialize("agent_data.db");
    auto& repoManager = agent::repository::RepositoryManager::getInstance();
    repoManager.settings().saveSettings(mySetting);
    Orchestrator orchestrator;
    orchestrator.onMessageReceived = [](const std::string& text, const Plan& plan) {
        cout << "LLM Response: " << text << endl;
        cout << "Plan: " << plan.name << endl << plan.description << endl;
        for (auto& step : plan.steps) {
            cout << "Step " << step.title << " : " << step.content << endl;
        }
    };
    orchestrator.onStatusChanged = [&orchestrator](const AgentStatus status) {
        cout << "> ";
        if (status == AgentStatus::Observing)
            cout << "Observing" << endl;
        else if (status == AgentStatus::Thinking)
            cout << "Thinking" << endl;
    };
    orchestrator.createNewChat();
    string userInput;
    while (cin >> userInput) {
        if (orchestrator.getStatus() == AgentStatus::Idle) {
            orchestrator.handleUserPrompt(userInput);
        }
    }
}
*/

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

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    string apiKey = "sk-OzgzQqIc8azSnEH9Lzn5EYx1mLabqH2tizw99nVWGdTD0KE3";
    string endpoint = "https://api.gapgpt.app/v1/chat/completions";
    agent::config::LLMProviderConfig myConfig = agent::config::LLMProviderConfig("1", "gapgpt", "nigger", endpoint, apiKey, agent::config::ApiFormat::OpenAICompatible);
    agent::settings::UserSettings mySetting = agent::settings::UserSettings("aliAndReza", "random", "i am not epstien");
    mySetting.addProvider(myConfig);
    mySetting.setActiveProviderId("1");

    agent::repository::DatabaseManager::getInstance().initialize("agent_data.db");
    auto& repoManager = agent::repository::RepositoryManager::getInstance();
    repoManager.settings().saveSettings(mySetting);

    auto orchestrator = std::make_shared<Orchestrator>();
    AgentBridge agentBridge(orchestrator);

    ChatListModel chatModel;
    NavigationController navController(&agentBridge);
    InputBoxController inputController(&agentBridge);
    InputBoxModel inputModel;

    QObject::connect(&chatModel, &ChatListModel::chatSelected,
                     &agentBridge, &AgentBridge::setActiveChat);

    QObject::connect(&agentBridge, &AgentBridge::chatSessionLoaded,
                     &chatModel, &ChatListModel::addAndSelectChat);

    QObject::connect(&agentBridge, &AgentBridge::chatsLoaded,
                     &chatModel, &ChatListModel::setChats);

    agentBridge.loadChatsFromRepository();

    if (chatModel.rowCount() == 0) {
        orchestrator->createNewChat();
    } else {
        chatModel.selectChat(0);
    }

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("agentBridge", &agentBridge);
    engine.rootContext()->setContextProperty("chatModel", &chatModel);
    engine.rootContext()->setContextProperty("navController", &navController);
    engine.rootContext()->setContextProperty("inputBoxController", &inputController);
    engine.rootContext()->setContextProperty("inputBoxModel", &inputModel);

    const QUrl url(QStringLiteral("qrc:/AccessibilityService/Front-end/Main.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);
    //engine.loadFromModule("AccessibilityService", "Main");

    return app.exec();
}