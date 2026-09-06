#include "Orchestrator.h"
#include "Actuation/ExecutionCallStack.h"
#include "Actuation/ActionDispatcher.h"
#include "Actuation/PermissionValidator.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include "systemPrompt.h"
#include "LLMSender/JsonSender.hpp"
#include "LLMSender/LLMReciever.hpp"
#include "Observation/Services/WorldStateBuilderService.h"

Orchestrator::Orchestrator()
    : repositoryManager(agent::repository::RepositoryManager::getInstance()) {
    loadUserSettings();
    createNewChat();
}

Orchestrator::~Orchestrator() {
    requestStop();
    waitForActiveTask();
}

void Orchestrator::loadUserSettings() {
    if (auto loadedSettings = repositoryManager.settings().getSettings()) {
        userSettings = std::make_shared<agent::settings::UserSettings>(loadedSettings.value());
    } else {
        throw std::runtime_error("Critical Error: Failed to load user settings from database.");
    }
}

void Orchestrator::waitForActiveTask() {
    if (activeWorker.valid()) {
        activeWorker.wait();
    }
}

// -----------------------------------------------------------------------------
// Utilities & Context Management
// -----------------------------------------------------------------------------

std::string Orchestrator::generateChatId() {
    static std::atomic<uint32_t> chatCounter{0};
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    std::ostringstream oss;
    oss << "chat_" << ms << "_" << ++chatCounter;
    return oss.str();
}

std::string Orchestrator::generateMessageId() {
    static std::atomic<uint32_t> msgCounter{0};
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    std::ostringstream oss;
    oss << "msg_" << ms << "_" << ++msgCounter;
    return oss.str();
}

void Orchestrator::appendContext(const std::string& newText) {
    //TODO
}

void Orchestrator::compressContext() {
    JsonSender sender;
    const agent::config::LLMProviderConfig config = getActiveConfig();
    const std::string apiKey = config.api_key();
    const std::string endpoint = config.base_url();

    const std::string result = sender.SendDataToLLM(
        apiKey,
        endpoint,
        currentChat->getContextWindow(),
        systemPrompt::compressContextPrompt,
        "",
        "",
        "",
        "gpt-4o"
    );
    currentChat->setContextWindow(result);
}

// -----------------------------------------------------------------------------
// Chat & Session Management
// -----------------------------------------------------------------------------

void Orchestrator::createNewChat() {
    std::string newId = generateChatId();
    currentChat = std::make_shared<agent::chat::ChatHistory>(newId, "New Conversation");
    activeCallStack = &currentChat->getMutableExecutionCallStack();

    repositoryManager.chat().saveHistory(*currentChat);

    if (onChatLoaded) {
        onChatLoaded(currentChat);
    }
}

bool Orchestrator::setActiveChat(const std::string& chatId) {
    if (currentStatus.load() != AgentStatus::Idle) {
        if (onError) onError("Cannot switch chats while the agent is active.");
        return false;
    }

    auto loadedChatOpt = repositoryManager.chat().getHistory(chatId);
    if (!loadedChatOpt) {
        if (onError) onError("Chat history not found.");
        return false;
    }

    auto messages = repositoryManager.chat().getMessagesForChat(loadedChatOpt->getId());
    loadedChatOpt->getMutableMessages() = std::move(messages);

    currentChat = std::move(loadedChatOpt);
    activeCallStack = &currentChat->getMutableExecutionCallStack();

    if (onChatLoaded) {
        onChatLoaded(currentChat);
    }

    return true;
}

// -----------------------------------------------------------------------------
// State Transitions
// -----------------------------------------------------------------------------

void Orchestrator::changeStatus(const AgentStatus newStatus) {
    currentStatus.store(newStatus);
    if (onStatusChanged) {
        onStatusChanged(newStatus);
    }
}

void Orchestrator::abortWorkflow(const std::string& reason) {
    changeStatus(AgentStatus::Error);

    if (onError) onError(reason);
    if (activeCallStack != nullptr) {
        activeCallStack->abort();
    }

    pendingAction.reset();

    changeStatus(AgentStatus::Idle);
}

void Orchestrator::requestStop() {
    cancelRequested.store(true);
    if (activeCallStack != nullptr) {
        activeCallStack->abort();
    }
}

// -----------------------------------------------------------------------------
// Phase 1: Observation
// -----------------------------------------------------------------------------

void Orchestrator::handleUserPrompt(const std::string& prompt) {
    if (currentStatus.load() != AgentStatus::Idle) {
        if (onError) onError("System is busy. Please wait.");
        return;
    }

    cancelRequested.store(false);
    lastUserPrompt = prompt;

    currentMessageId = generateMessageId();
    agent::chat::Message newMsg(currentMessageId, prompt, "");
    newMsg.setChatId(currentChat->getId());

    currentChat->addMessage(newMsg);
    repositoryManager.chat().saveMessage(newMsg);

    //appendContext("User: " + prompt);

    changeStatus(AgentStatus::Observing);
    //debug:
    cout << "> Observing" << endl;
    triggerObservationAsync();
}

void Orchestrator::triggerObservationAsync(ObservationFlags flags) {
    activeWorker = std::async(std::launch::async, [this, flags]() {
        if (cancelRequested.load()) return;

        auto& worldStateBuilder = WorldStateBuilderService::getInstance();
        worldStateBuilder.observe(flags);
        std::shared_ptr<const WorldState> state = std::make_shared<WorldState>(worldStateBuilder.consumeState());

        onObservationCompleted(state);
    });
}

void Orchestrator::onObservationCompleted(std::shared_ptr<const WorldState> state) {
    if (cancelRequested.load()) {
        abortWorkflow("Operation cancelled during observation.");
        return;
    }

    currentWorldState = state;
    changeStatus(AgentStatus::Thinking);
    //debug:
    cout << "> Thinking" << endl;
    triggerThinkingAsync();
}

// -----------------------------------------------------------------------------
// Phase 2: LLM Interaction
// -----------------------------------------------------------------------------

agent::config::LLMProviderConfig Orchestrator::getActiveConfig() {
    const std::string& id = userSettings->activeProviderId();
    //debug:
    cout << "> Getting Config" << endl;
    return userSettings->getProvider(id).value();
}

void Orchestrator::triggerThinkingAsync() {
    // Already running inside activeWorker thread, do not re-wrap in std::async
    if (cancelRequested.load()) return;

    JsonSender sender;
    std::string finalPromptContext = currentChat->getContextWindow();
    const agent::config::LLMProviderConfig config = getActiveConfig();
    const std::string apiKey = config.api_key();
    const std::string endpoint = config.base_url();

    //TODO: append context window to sedning payload to llm
    json tools = Actions::BuildToolsSchema();
    std::string result = sender.SendDataToLLM(
        apiKey,
        endpoint,
        lastUserPrompt,
        systemPrompt::sysData,
        tools,
        "",
        "",
        "gpt-4o"
    );
    //debug:
    cout << "> Message Recieved" << endl;
    onLlmResponseReady(result);
}

void Orchestrator::onLlmResponseReady(const std::string& rawResponse) {
    if (cancelRequested.load()) {
        abortWorkflow("Operation cancelled during LLM response.");
        return;
    }

    //debug:
    cout << "> Parsing" << endl;
    cout << "Raw Response: " << rawResponse << endl;
    processLlmResponse(rawResponse);
}

// -----------------------------------------------------------------------------
// Phase 3: Parsing
// -----------------------------------------------------------------------------

void Orchestrator::processLlmResponse(const std::string& rawResponse) {
    //appendContext("Assistant: " + rawResponse);

    if (auto* lastMsg = currentChat->getLastMessage()) {
        repositoryManager.chat().saveMessage(*lastMsg);
    }
    repositoryManager.chat().saveHistory(*currentChat);

    Plan tempPlan;
    std::string messageToUser;

    if (activeCallStack != nullptr) {
        LLMReciever::getInstance().parse(rawResponse, *activeCallStack, tempPlan, messageToUser);
    }
    currentChat->updateLastMessageResult(rawResponse, messageToUser, tempPlan);

    if (!messageToUser.empty() && onMessageReceived) {
        onMessageReceived(messageToUser, tempPlan);
        cout << "Result: " << messageToUser << endl;
        cout << "Plan: " << tempPlan.name << endl << tempPlan.description << endl;
        for (auto& step : tempPlan.steps) {
            cout << "Step " << step.title << " : " << step.content << endl;
        }
    }

    // Bypass batch approval; proceed directly to step-by-step JIT execution
    changeStatus(AgentStatus::Executing);
    //executeNextActionAsync();
}

// -----------------------------------------------------------------------------
// Phase 4: Actuation, Validation & Feedback
// -----------------------------------------------------------------------------

void Orchestrator::executeNextActionAsync() {
    // 1. Fetch next action if we don't have one pending
    if (!pendingAction.has_value()) {
        if (activeCallStack == nullptr || activeCallStack->isEmpty()) {
            changeStatus(AgentStatus::Idle);
            if (onTaskCompleted) onTaskCompleted();
            return;
        }

        std::optional<ActionItem> optAction = activeCallStack->getNextAction();
        if (!optAction.has_value()) {
            changeStatus(AgentStatus::Idle);
            if (onTaskCompleted) onTaskCompleted();
            return;
        }

        pendingAction = optAction;
    }

    // 2. Run Permission Validation Filter
    ActionItem currentAction = pendingAction.value();
    PermissionLevel permLevel = PermissionValidator::validate(currentAction.payload);

    if (permLevel == PermissionLevel::RequiresConfirmation) {
        changeStatus(AgentStatus::WaitingForApproval);
        if (onApprovalRequested) {
            onApprovalRequested("Action requires confirmation: " + currentAction.action_id);
        }
        return; // Halt and wait for user to call handleUserApproval()
    }
    else if (permLevel == PermissionLevel::Denied) {
        pendingAction.reset(); // Drop the denied action
        triggerReplanningAsync("Action denied due to strict permission policy.");
        return;
    }

    // 3. If Allowed, dispatch
    dispatchPendingActionAsync();
}

void Orchestrator::handleUserApproval(bool isApproved) {
    if (currentStatus.load() != AgentStatus::WaitingForApproval) return;

    if (!isApproved) {
        pendingAction.reset();
        abortWorkflow("Action execution declined by user.");
        return;
    }

    // User approved, proceed with dispatching the paused action on a background worker
    changeStatus(AgentStatus::Executing);
    activeWorker = std::async(std::launch::async, [this]() {
        dispatchPendingActionAsync();
    });
}

void Orchestrator::dispatchPendingActionAsync() {
    ActionItem currentAction = pendingAction.value();
    pendingAction.reset();

    if (cancelRequested.load()) {
        if (activeCallStack) activeCallStack->abort();
        return;
    }
    if (std::holds_alternative<Actions::ControlData>(currentAction.payload)) {
        auto& controlData = std::get<Actions::ControlData>(currentAction.payload);
        if (std::holds_alternative<Actions::Observe>(controlData)){
            auto& observationRequest = std::get<Actions::Observe>(controlData);
            auto& worldStateBuilder = WorldStateBuilderService::getInstance();
            worldStateBuilder.observe(observationRequest.flags);
            currentWorldState = std::make_shared<WorldState>(worldStateBuilder.consumeState());
        }
    }
    ActionStatus status = ActionDispatcher::dispatch(currentAction.payload);
    handleActionResult(status);
}

void Orchestrator::handleActionResult(ActionStatus status) {
    switch (status) {
        case ActionStatus::Success:
            executeNextActionAsync();
            break;

        case ActionStatus::TriggerObserve:
            changeStatus(AgentStatus::Observing);
            triggerObservationAsync();
            break;

        case ActionStatus::Failed:
            triggerReplanningAsync("Action execution failed.");
            break;
    }
}

void Orchestrator::triggerReplanningAsync(const std::string& failureReason) {
    //appendContext("System Note: " + failureReason + " Please replan.");
    changeStatus(AgentStatus::Thinking);
    const ObservationFlags actionFailureFlags = ObservationFlags{true, false,
        false, false,
        "", false, false, false};

    // triggerObservationAsync naturally calls triggerThinkingAsync once observation completes
    triggerObservationAsync(actionFailureFlags);
}