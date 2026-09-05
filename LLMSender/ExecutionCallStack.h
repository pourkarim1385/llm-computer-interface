#pragma once

#include "Actions.h"
#include <deque>
#include <optional>
#include <mutex>
#include <condition_variable>
#include <string>
#include <span>

struct Step {
    std::string title;
    std::string content;
    bool isDone{false};
};

struct Plan {
    std::string name;
    std::string description;
    std::vector<Step> steps;
};

struct ActionItem {
    std::string action_id;
    std::string sequence_id;
    Actions::Action payload;
};

class ExecutionCallStack {
public:
    ExecutionCallStack() = default;
    ~ExecutionCallStack() = default;

    // Prevent copying to avoid accidental deep copies of the execution state
    ExecutionCallStack(const ExecutionCallStack&) = delete;
    ExecutionCallStack& operator=(const ExecutionCallStack&) = delete;

    // Pushes a single action to the top of the stack
    void push(ActionItem item);
    void push_back(ActionItem item);

    // Pushes a sequence of actions (useful when parsing a block of LLM output)
    void pushSequence(std::span<const ActionItem> actions);

    // Retrieves and removes the next action to execute
    std::optional<ActionItem> getNextAction();

    // Inspects the next action without removing it
    std::optional<ActionItem> peek() const;
    bool modifyAction(const std::string& action_id, Actions::Action new_payload);

    void clear();
    size_t size() const;
    bool isEmpty() const;

    std::string toJsonSummary() const;

    void abort();
private:
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
    std::deque<ActionItem> m_actions;
    bool m_abort = false;
};