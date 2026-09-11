#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <functional>
#include <future>
#include <chrono>
#include <optional>
#include <mutex>
#include <condition_variable>
#include "Context/ChatHistory.h"
#include "Repository/RepositoryManager.h"
#include "Actuation/ExecutionCallStack.h"
#include "Observation/Services/WorldStateBuilderService.h"

class WorldState;
class ActionDispatcher;

enum class AgentStatus {
    Idle,
    Observing,
    Thinking,
    WaitingForApproval,
    Executing,
    Error
};

/**
 * Orchestrator coordinates the agent's core loop, handling background tasks,
 * state machine transitions, context management, and user interactions.
 */
class Orchestrator {
public:
    Orchestrator();
    ~Orchestrator();

    Orchestrator(const Orchestrator&) = delete;
    Orchestrator& operator=(const Orchestrator&) = delete;

    /**
     * Inbound Events from UI / Gateway
     */
    void handleUserPrompt(const std::string& prompt);
    void handleUserApproval(bool isApproved);
    void requestStop();

    /**
     * Chat Management
     */
    bool setActiveChat(const std::string& chatId);
    void createNewChat();

    /**
     * Outbound Callbacks for UI integration
     */
    std::function<void(AgentStatus)> onStatusChanged;
    std::function<void(const std::string& description)> onApprovalRequested;
    std::function<void(const std::string& errorMsg)> onError;
    std::function<void()> onTaskCompleted;
    std::function<void(std::shared_ptr<agent::chat::ChatHistory>)> onChatLoaded;
    std::function<void(const std::string& message, const Plan& plan)> onMessageReceived;

    AgentStatus getStatus() const { return currentStatus.load(); }

private:
    std::atomic<bool> cancelRequested{false};
    std::mutex cancelMutex;
    std::condition_variable cancelCv;

    bool interruptibleSleep(int milliseconds);

    void loadUserSettings();
    agent::config::LLMProviderConfig getActiveConfig();
    int64_t getTimestamp();

    /**
     * Core State Management
     */
    void changeStatus(AgentStatus newStatus);
    void abortWorkflow(const std::string& reason);
    void waitForActiveTask();

    /**
     * Context & Window Management
     */
    void appendContext(const std::string& newText);
    void compressContext();

    /**
     * Unique Identifier Generators
     */
    std::string generateChatId();
    std::string generateMessageId();

    /**
     * Workflow Phases
     */
    void runObservation(ObservationFlags flags);
    void triggerObservationAsync(ObservationFlags flags = ObservationFlags{});
    void onObservationCompleted(std::shared_ptr<WorldState> state);

    void triggerThinkingAsync();
    void onLlmResponseReady(const std::string& rawResponse);

    void processLlmResponse(const std::string& rawResponse);

    // JIT Execution Pipeline
    void executeNextActionAsync();
    void dispatchPendingActionAsync();

    void handleActionResult(ActionStatus status, const ActionItem& executedAction);
    void triggerReplanningAsync(const std::string& failureReason);

    void commitAssistantMessage(const std::string& rawResponse, const std::string& newChunk, const Plan& plan);

    /**
     * Member Variables
     */
    std::atomic<AgentStatus> currentStatus{AgentStatus::Idle};

    agent::repository::RepositoryManager& repositoryManager;

    std::shared_ptr<agent::chat::ChatHistory> currentChat;
    std::shared_ptr<agent::settings::UserSettings> userSettings;
    std::shared_ptr<WorldState> currentWorldState;
    ExecutionCallStack* activeCallStack{nullptr};

    // Temporarily holds the action awaiting Permission Validation or UI Approval
    std::optional<ActionItem> pendingAction;

    std::string lastUserPrompt;
    std::string currentMessageId;

    std::future<void> activeWorker;

    int currentActionRetryCount{0};
    static constexpr int MAX_ACTION_RETRIES = 2;

    int currentTurnCount{0};
    static constexpr int MAX_TURNS = 15;
};