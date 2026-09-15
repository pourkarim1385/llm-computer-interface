#include "Orchestrator.h"
#include "Actuation/ExecutionCallStack.h"
#include "Actuation/ActionDispatcher.h"
#include "Actuation/PermissionValidator.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <algorithm>
#include "systemPrompt.h"
#include "LlmGateway/Tools.hpp"
#include "LlmGateway/JsonSender.hpp"
#include "LlmGateway/LLMReciever.hpp"
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
        if (!userSettings) {
            throw std::runtime_error("Critical Error: Failed to load user settings from database.");
        }
        std::cerr << "[Orchestrator] Warning: Failed to reload settings from DB, keeping current memory state.\n";
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
    // TODO: Context appending
}

static std::string stripMarkdownFences(const std::string& input) {
    std::string s = input;
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    s = s.substr(start, end - start + 1);

    if (s.rfind("```", 0) == 0) {
        size_t firstNewline = s.find('\n');
        if (firstNewline != std::string::npos) {
            s = s.substr(firstNewline + 1);
        }
        if (s.length() >= 3 && s.substr(s.length() - 3) == "```") {
            s = s.substr(0, s.length() - 3);
        }
    }
    return s;
}

void Orchestrator::compressContext() {
    if (!currentChat) return;

    auto& memory = currentChat->getMutableMemory();

    if (!memory.shouldCompressGoals() &&
        !memory.shouldCompressFacts() &&
        !memory.shouldCompressFiles()) {
        return;
    }

    auto executeCompression = [this](const std::string& payload, const std::string& sysPrompt) -> std::string {
        JsonSender sender;
        const agent::config::LLMProviderConfig config = getActiveConfig();
        const std::string apiKey = config.api_key();
        const std::string endpoint = config.base_url();
        const std::string model = config.model_id();

        std::string rawResponse = sender.sendDataToLLM(
            apiKey,
            endpoint,
            payload,
            sysPrompt,
            model,
            0.1
        );

        if (rawResponse.empty()) {
            std::cerr << "[ContextCompression] Empty response received from LLM." << std::endl;
            return "";
        }

        try {
            auto jsonResponse = nlohmann::json::parse(rawResponse);
            if (jsonResponse.contains("choices") && !jsonResponse["choices"].empty()) {
                return jsonResponse["choices"][0]["message"]["content"];
            }
        } catch (const std::exception& e) {
            std::cerr << "[ContextCompression Error] Failed to parse LLM envelope: " << e.what() << std::endl;
        }
        return "";
    };

    bool memoryChanged = false;

    if (memory.shouldCompressGoals()) {
        nlohmann::json root;
        root["current_goals"] = memory.getGoals();
        std::string payload = "Please compress and summarize these goals into the requested schema:\n" + root.dump(2);

        std::string response = executeCompression(payload, systemPrompt::compressGoalsPrompt);
        if (!response.empty()) {
            try {
                auto parsed = nlohmann::json::parse(stripMarkdownFences(response));
                if (parsed.contains("goals") && parsed["goals"].is_array()) {
                    std::vector<std::string> newGoals;
                    for (const auto& item : parsed["goals"]) {
                        if (item.is_string() && !item.get<std::string>().empty()) {
                            newGoals.push_back(item.get<std::string>());
                        }
                    }
                    if (!newGoals.empty()) {
                        memory.setCompressedGoals(std::move(newGoals));
                        memoryChanged = true;
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "[ContextCompression] JSON parse error in Goals: " << e.what() << std::endl;
            }
        }
    }

    if (memory.shouldCompressFacts()) {
        nlohmann::json root;
        root["current_facts"] = memory.getEnvFacts();
        std::string payload = "Please deduplicate and compact these environment facts into the requested schema:\n" + root.dump(2);

        std::string response = executeCompression(payload, systemPrompt::compressFactsPrompt);
        if (!response.empty()) {
            try {
                auto parsed = nlohmann::json::parse(stripMarkdownFences(response));
                if (parsed.contains("facts") && parsed["facts"].is_array()) {
                    std::vector<std::string> newFacts;
                    for (const auto& item : parsed["facts"]) {
                        if (item.is_string() && !item.get<std::string>().empty()) {
                            newFacts.push_back(item.get<std::string>());
                        }
                    }
                    if (!newFacts.empty()) {
                        memory.setCompressedFacts(std::move(newFacts));
                        memoryChanged = true;
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "[ContextCompression] JSON parse error in Facts: " << e.what() << std::endl;
            }
        }
    }

    if (memory.shouldCompressFiles()) {
        nlohmann::json root;
        root["current_files"] = memory.getFileInsights();
        std::string payload = "Please compress the summaries of these file insights into the requested schema:\n" + root.dump(2);

        std::string response = executeCompression(payload, systemPrompt::compressFilesPrompt);
        if (!response.empty()) {
            try {
                auto parsed = nlohmann::json::parse(stripMarkdownFences(response));
                if (parsed.contains("files") && parsed["files"].is_object()) {
                    std::unordered_map<std::string, std::string> newFiles;
                    for (auto it = parsed["files"].begin(); it != parsed["files"].end(); ++it) {
                        if (it.value().is_string() && !it.value().get<std::string>().empty()) {
                            newFiles[it.key()] = it.value().get<std::string>();
                        }
                    }
                    if (!newFiles.empty()) {
                        memory.setFileInsights(std::move(newFiles));
                        memoryChanged = true;
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "[ContextCompression] JSON parse error in Files: " << e.what() << std::endl;
            }
        }
    }

    if (memoryChanged) {
        repositoryManager.chat().saveHistory(*currentChat);
        std::cout << "[Orchestrator] ChatMemory was compressed and persisted successfully." << std::endl;
    }
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

    currentChat = std::move(loadedChatOpt);
    activeCallStack = &currentChat->getMutableExecutionCallStack();

    if (onChatLoaded) {
        onChatLoaded(currentChat);
    }

    std::cout << "Current Chat set to: " << currentChat->getId() << std::endl;
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
    std::cout << "Workflow aborted: " << reason << std::endl;
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
    cancelCv.notify_all();
    if (activeCallStack != nullptr) {
        activeCallStack->abort();
    }
}

// -----------------------------------------------------------------------------
// Phase 1: Observation
// -----------------------------------------------------------------------------

void Orchestrator::handleUserPrompt(const std::string& prompt, const ObservationFlags &flags) {
    if (currentStatus.load() != AgentStatus::Idle || !currentChat) {
        if (onError) onError("System is busy. Please wait.");
        return;
    }

    cancelRequested.store(false);
    currentActionRetryCount = 0;
    currentTurnCount = 0;
    lastUserPrompt = prompt;
    currentChat->setLastModifiedAtUnixSec(getTimestamp());

    currentMessageId = generateMessageId();
    agent::chat::Message newMsg(currentMessageId, prompt, "");
    newMsg.setChatId(currentChat->getId());

    currentChat->addMessage(newMsg);
    repositoryManager.chat().saveMessage(newMsg);

    currentChat->appendCurrentTaskHistory("[User entered Prompt]: " + prompt + ";\n");

    changeStatus(AgentStatus::Observing);
    std::cout << "> Observing" << std::endl;
    triggerObservationAsync(flags);
}

void Orchestrator::runObservation(const ObservationFlags &flags) {
    if (cancelRequested.load()) {
        abortWorkflow("Operation cancelled before observation.");
        return;
    }

    auto& worldStateBuilder = WorldStateBuilderService::getInstance();
    worldStateBuilder.observe(flags);
    std::shared_ptr<WorldState> state = std::make_shared<WorldState>(worldStateBuilder.consumeState());

    onObservationCompleted(state);
}

void Orchestrator::triggerObservationAsync(const ObservationFlags& flags) {
    activeWorker = std::async(std::launch::async, [this, flags]() {
        runObservation(flags);
    });
}

void Orchestrator::onObservationCompleted(std::shared_ptr<WorldState> state) {
    if (cancelRequested.load()) {
        abortWorkflow("Operation cancelled during observation.");
        return;
    }

    currentWorldState = state;
    changeStatus(AgentStatus::Thinking);
    std::cout << "> Thinking" << std::endl;
    triggerThinkingAsync();
}

// -----------------------------------------------------------------------------
// Phase 2: LLM Interaction
// -----------------------------------------------------------------------------

bool Orchestrator::reloadSettings() {
    std::lock_guard<std::mutex> lock(settingsMutex);
    try {
        loadUserSettings();
        std::cout << "[Orchestrator] Settings reloaded successfully." << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "[Orchestrator] Failed to reload settings: " << e.what()
                  << " | Retaining previous settings in memory." << std::endl;

        if (onError) onError(std::string("Settings sync failed: ") + e.what());
        return false;
    }
    catch (...) {
        std::cerr << "[Orchestrator] Unknown error occurred while reloading settings." << std::endl;
        if (onError) onError(std::string("Settings sync failed"));
        return false;
    }
}

agent::config::LLMProviderConfig Orchestrator::getActiveConfig() {
    std::lock_guard<std::mutex> lock(settingsMutex);
    auto activeProv = userSettings->getActiveProvider();
    if (activeProv.has_value()) {
        return activeProv.value();
    }

    if (!userSettings->providers().empty()) {
        return userSettings->providers().front();
    }

    return agent::config::LLMProviderConfig("dummy", "Fallback", "", "", "", agent::config::ApiFormat::OpenAICompatible);
}

void Orchestrator::triggerThinkingAsync() {
    if (cancelRequested.load()) return;

    compressContext();

    JsonSender sender;
    const agent::config::LLMProviderConfig config = getActiveConfig();
    const std::string& apiKey = config.api_key();
    const std::string& endpoint = config.base_url();
    const std::string& model = config.model_id();

    std::string promptForLLM = "### Current User Goal:\n" + lastUserPrompt + "\n";

    if (currentChat && !currentChat->getMemory().empty()) {
        promptForLLM += "### Active Working Memory & Cache:\n" + currentChat->getMemory().toMarkdown() + "\n\n";
    }

    if (!currentChat->getCurrentTaskHistory().empty()) {
        promptForLLM += "\n\n### Executed Actions Trajectory in Current Task:\n" + currentChat->getCurrentTaskHistory() +
                        "\nReview the trajectory above and latest WorldState to plan the next actions or complete the goal.";
    }

    json tools = BuildToolsSchema();
    double temp = 1.0;

    std::string result = sender.sendDataToLLM(
        apiKey,
        endpoint,
        promptForLLM,
        systemPrompt::sysData,
        tools,
        currentWorldState,
        model,
        temp
    );

    onLlmResponseReady(result);
}

void Orchestrator::onLlmResponseReady(const std::string& rawResponse) {
    if (cancelRequested.load()) {
        abortWorkflow("Operation cancelled during LLM response.");
        return;
    }

    std::cout << "Raw response: " << rawResponse << std::endl;
    std::cout << "> Parsing" << std::endl;
    processLlmResponse(rawResponse);
}

// -----------------------------------------------------------------------------
// Phase 3: Parsing
// -----------------------------------------------------------------------------

void Orchestrator::commitAssistantMessage(const std::string& rawResponse,
                                         const std::string& newChunk,
                                         const Plan& plan) {
    if (!currentChat) return;

    std::string accumulated = newChunk;
    if (auto* lastMsg = currentChat->getLastMessage()) {
        const std::string& prev = lastMsg->getResult();
        if (!prev.empty() && !newChunk.empty()) {
            accumulated = prev + "\n\n---\n\n" + newChunk;
        } else if (newChunk.empty()) {
            accumulated = prev;
        }
    }

    currentChat->updateLastMessageResult(rawResponse, accumulated, plan);
    if (auto* lastMsg = currentChat->getLastMessage()) {
        repositoryManager.chat().saveMessage(*lastMsg);
    }
    repositoryManager.chat().saveHistory(*currentChat);

    if (onMessageReceived) {
        onMessageReceived(accumulated, plan);
    }
}

void Orchestrator::processLlmResponse(const std::string& rawResponse) {
    currentTurnCount++;
    if (currentTurnCount > MAX_TURNS) {
        abortWorkflow("Execution stopped: Reached maximum turn limit (" + std::to_string(MAX_TURNS) + ").");
        return;
    }

    Plan plan;
    std::string messageToUser;

    try {
        if (activeCallStack != nullptr && currentChat != nullptr) {
            LLMReciever::getInstance().parse(rawResponse, *activeCallStack, plan, messageToUser, currentChat->getMutableMemory());

            if (!activeCallStack->isEmpty()) {
                activeCallStack->push_back(ActionItem(
                    "EndOfStackObservation",
                    "EndOfStackObservation",
                    Actions::Observe{ObservationFlags{true, false, false, false, "", false, true, false}}
                ));
            }
        }
    }
    catch (const LLMException& e) {
        Plan existingPlan = currentChat ? currentChat->getPlan() : Plan{};

        switch (e.getErrorType()) {
            case LLMErrorType::ParseAction:
            case LLMErrorType::ProtocolViolation: {
                std::cout << "[LLM Recoverable Error] " << e.what()
                          << " -> Triggering Replanning." << std::endl;

                if (activeCallStack != nullptr) {
                    activeCallStack->clear();
                }

                if (currentChat) {
                    currentChat->appendCurrentTaskHistory(
                        "[System Feedback - Invalid Plan Detected]: " + std::string(e.what()) +
                        "\nAction Required: Correct your plan. Only choose valid tools from the specification with correct arguments, and strictly follow the protocol schema.\n"
                    );
                }

                triggerReplanningAsync("Plan correction needed: " + e.getReason());
                return;
            }

            case LLMErrorType::ApiError:
            case LLMErrorType::Validation:
            default: {
                std::cerr << "[LLM Fatal Error] " << e.what() << " -> Aborting Workflow." << std::endl;

                if (activeCallStack != nullptr) {
                    activeCallStack->clear();
                }

                std::string userMessage;
                if (e.getErrorType() == LLMErrorType::ApiError) {
                    userMessage = "⚠️ **ProviderService Response (API Error):**\n" + e.getReason();
                } else {
                    userMessage = "⚠️ **Connection Error Or Wrong Response Format:**\n" + e.getReason();
                }

                commitAssistantMessage(rawResponse, userMessage, existingPlan);

                abortWorkflow(e.what());
                return;
            }
        }
    }
    catch (const std::exception& e) {
        Plan existingPlan = currentChat ? currentChat->getPlan() : Plan{};
        commitAssistantMessage(rawResponse, "⚠️ **Execution Error:** " + std::string(e.what()), existingPlan);
        abortWorkflow(e.what());
        return;
    }
    catch (...) {
        Plan existingPlan = currentChat ? currentChat->getPlan() : Plan{};
        commitAssistantMessage(rawResponse, "⚠️ **Execution Error:** Unknown Error", existingPlan);
        abortWorkflow("Unknown Error");
        return;
    }

    commitAssistantMessage(rawResponse, messageToUser, plan);

    if (activeCallStack != nullptr && !activeCallStack->isEmpty()) {
        changeStatus(AgentStatus::Executing);
        executeNextActionAsync();
    } else {
        changeStatus(AgentStatus::Idle);
        currentTurnCount = 0;
        if (currentChat) currentChat->setCurrentTaskHistory("");
        if (onTaskCompleted) onTaskCompleted();
    }
}

// -----------------------------------------------------------------------------
// Phase 4: Actuation, Validation & Feedback
// -----------------------------------------------------------------------------

void Orchestrator::executeNextActionAsync() {
    if (!pendingAction.has_value()) {
        if (activeCallStack == nullptr || activeCallStack->isEmpty()) {
            changeStatus(AgentStatus::Idle);
            currentChat->setCurrentTaskHistory("");
            if (onTaskCompleted) onTaskCompleted();
            return;
        }

        std::optional<ActionItem> optAction = activeCallStack->peek();
        if (!optAction.has_value()) {
            changeStatus(AgentStatus::Idle);
            currentChat->setCurrentTaskHistory("");
            if (onTaskCompleted) onTaskCompleted();
            return;
        }

        pendingAction = optAction;
    }

    ActionItem currentAction = pendingAction.value();
    PermissionLevel permLevel = PermissionValidator::validate(currentAction.payload);

    if (permLevel == PermissionLevel::RequiresConfirmation) {
        changeStatus(AgentStatus::WaitingForApproval);
        if (onApprovalRequested) {
            onApprovalRequested("Action requires confirmation: " + currentAction.action_id);
        }
        return;
    }
    else if (permLevel == PermissionLevel::Denied) {
        pendingAction.reset();
        if (activeCallStack != nullptr) {
            activeCallStack->clear();
        }
        currentChat->appendCurrentTaskHistory("Execution Blocked: Action " + currentAction.action_id + " was denied by policy.\n");
        triggerReplanningAsync("Action denied due to strict permission policy.");
        return;
    }

    dispatchPendingActionAsync();
}

void Orchestrator::handleUserApproval(bool isApproved) {
    if (currentStatus.load() != AgentStatus::WaitingForApproval) return;

    if (!isApproved) {
        pendingAction.reset();
        abortWorkflow("Action execution declined by user.");
        return;
    }

    changeStatus(AgentStatus::Executing);
    activeWorker = std::async(std::launch::async, [this]() {
        dispatchPendingActionAsync();
    });
}

void Orchestrator::dispatchPendingActionAsync() {
    if (!pendingAction.has_value()) return;

    ActionItem currentAction = pendingAction.value();
    pendingAction.reset();

    if (cancelRequested.load()) {
        if (activeCallStack) activeCallStack->abort();
        abortWorkflow("Execution stopped by user.");
        return;
    }

    if (std::holds_alternative<Actions::ControlData>(currentAction.payload)) {
        auto& controlData = std::get<Actions::ControlData>(currentAction.payload);

        if (std::holds_alternative<Actions::SearchWeb>(controlData)) {
            auto& searchWeb = std::get<Actions::SearchWeb>(controlData);
            std::lock_guard<std::mutex> lock(settingsMutex);
            searchWeb.config = userSettings->getSearchProviderConfig();
        }
        else if (std::holds_alternative<Actions::ClearStack>(controlData)) {
            if (activeCallStack) activeCallStack->clear();
        }
        else if (std::holds_alternative<Actions::Wait>(controlData)) {
            auto& waitAction = std::get<Actions::Wait>(controlData);
            bool completed = interruptibleSleep(waitAction.value);

            if (!completed) {
                abortWorkflow("Wait cancelled by user.");
                return;
            }

            WorldStateBuilderService::getInstance().pushActionResult(
                "[Wait] Slept for " + std::to_string(waitAction.value) + " ms."
            );
        }
    }

    ActionStatus status = ActionDispatcher::dispatch(currentAction.payload);
    handleActionResult(status, currentAction);
}

void Orchestrator::handleActionResult(ActionStatus status, const ActionItem& executedAction) {
    auto popIfMatches = [this, &executedAction]() {
        if (activeCallStack && !activeCallStack->isEmpty()) {
            auto top = activeCallStack->peek();
            if (top && top->action_id == executedAction.action_id) {
                activeCallStack->getNextAction();
            }
        }
    };

    switch (status) {
        case ActionStatus::Ok: {
            currentActionRetryCount = 0;
            popIfMatches();

            currentChat->appendCurrentTaskHistory("Successfully Executed: ActionID: " + executedAction.action_id +
                                  "; SequenceId: " + executedAction.sequence_id +
                                  "; Content: " + ActionDispatcher::actionToString(executedAction.payload) + ";\n");
            executeNextActionAsync();
            break;
        }

        case ActionStatus::TriggerObserve: {
            currentActionRetryCount = 0;
            popIfMatches();

            currentChat->appendCurrentTaskHistory("Successfully Executed: ActionID: " + executedAction.action_id +
                                  "; SequenceId: " + executedAction.sequence_id +
                                  "; Content: " + ActionDispatcher::actionToString(executedAction.payload) + ";\n");

            ObservationFlags flags{};
            if (std::holds_alternative<Actions::ControlData>(executedAction.payload)) {
                const auto& cd = std::get<Actions::ControlData>(executedAction.payload);
                if (std::holds_alternative<Actions::Observe>(cd)) {
                    flags = std::get<Actions::Observe>(cd).flags;
                }
            }

            changeStatus(AgentStatus::Observing);
            runObservation(flags);
            break;
        }

        case ActionStatus::Failed: {
            currentActionRetryCount++;
            std::string actionInfo = "ActionID: " + executedAction.action_id +
                                     "; Content: " + ActionDispatcher::actionToString(executedAction.payload);

            if (currentActionRetryCount < MAX_ACTION_RETRIES) {
                currentChat->appendCurrentTaskHistory("Execution Failed (Attempt " + std::to_string(currentActionRetryCount) +
                                      "/" + std::to_string(MAX_ACTION_RETRIES) + "): " + actionInfo + " -> Retrying...\n");

                std::cout << "[Retry] Retrying failed action (Attempt " << currentActionRetryCount << ")" << std::endl;
                executeNextActionAsync();
            }
            else {
                currentActionRetryCount = 0;
                currentChat->appendCurrentTaskHistory("Execution Failed permanently after " + std::to_string(MAX_ACTION_RETRIES) +
                                      " attempts: " + actionInfo + " -> Triggering Replanning.\n");

                if (activeCallStack != nullptr) {
                    activeCallStack->clear();
                }

                triggerReplanningAsync("Action execution failed after " + std::to_string(MAX_ACTION_RETRIES) + " attempts.");
            }
            break;
        }
    }
}

void Orchestrator::triggerReplanningAsync(const std::string& failureReason) {
    changeStatus(AgentStatus::Thinking);
    const ObservationFlags actionFailureFlags = ObservationFlags{true, false,
        false, false,
        "", false, false, false};

    runObservation(actionFailureFlags);
}

int64_t Orchestrator::getTimestamp() {
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

bool Orchestrator::interruptibleSleep(int milliseconds) {
    int waitMs = std::clamp(milliseconds, 0, 30000);
    if (waitMs <= 0) return true;

    std::unique_lock<std::mutex> lock(cancelMutex);

    cancelCv.wait_for(lock, std::chrono::milliseconds(waitMs), [this]() {
        return cancelRequested.load();
    });

    return !cancelRequested.load();
}