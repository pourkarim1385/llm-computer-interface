#include <iostream>

#include "Core/Orchestrator.h"
#include "Repository/DatabaseManager.h"

using namespace std;

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
    //orchestrator.onStatusChanged = [&orchestrator]() {
    //    cout << "> ";
    //    AgentStatus status = orchestrator.getStatus();
    //    if (status == AgentStatus::Observing)
    //        cout << "Observing" << endl;
    //    else if (status == AgentStatus::Thinking)
    //        cout << "Thinking" << endl;
    //};
    orchestrator.createNewChat();
    string userInput;
    while (cin >> userInput) {
        if (orchestrator.getStatus() == AgentStatus::Idle) {
            orchestrator.handleUserPrompt(userInput);
        }
    }
}