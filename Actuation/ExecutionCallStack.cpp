#include "ExecutionCallStack.h"

void ExecutionCallStack::push(ActionItem item) {
    {
        std::scoped_lock lock(m_mutex);
        m_actions.push_front(std::move(item));
    }
    m_cv.notify_one();
}

void ExecutionCallStack::push_back(ActionItem item) {
    {
        std::scoped_lock lock(m_mutex);
        m_actions.push_back(std::move(item));
    }
    m_cv.notify_one();
}

void ExecutionCallStack::pushSequence(std::span<const ActionItem> actions) {
    {
        std::scoped_lock lock(m_mutex);
        for (auto it = actions.rbegin(); it != actions.rend(); ++it) {
            m_actions.push_front(*it);
        }
    }
    m_cv.notify_one(); // Wakes up getNextAction()
}

std::optional<ActionItem> ExecutionCallStack::getNextAction() {
    std::unique_lock lock(m_mutex);

    // Wait until the queue is NOT empty OR an abort is signaled
    m_cv.wait(lock, [this] { return !m_actions.empty() || m_abort; });

    if (m_abort && m_actions.empty()) {
        return std::nullopt; // Safely tell the dispatcher to stop
    }

    ActionItem next_action = std::move(m_actions.front());
    m_actions.pop_front();
    return next_action;
}

bool  ExecutionCallStack::modifyAction(const std::string& action_id, Actions::Action new_payload) {
    std::scoped_lock lock(m_mutex);
    for (auto& item : m_actions) {
        if (item.action_id == action_id) {
            item.payload = std::move(new_payload);
            return true;
        }
    }
    return false;
}

std::optional<ActionItem> ExecutionCallStack::peek() const {
    std::scoped_lock lock(m_mutex);
    if (m_actions.empty()) {
        return std::nullopt;
    }
    return m_actions.front();
}

void ExecutionCallStack::clear() {
    std::scoped_lock lock(m_mutex);
    m_actions.clear();
}

size_t ExecutionCallStack::size() const {
    std::scoped_lock lock(m_mutex);
    return m_actions.size();
}

bool ExecutionCallStack::isEmpty() const {
    std::scoped_lock lock(m_mutex);
    return m_actions.empty();
}

std::string ExecutionCallStack::toJsonSummary() const {
    std::scoped_lock lock(m_mutex);
    std::string json = "[";
    for (size_t i = 0; i < m_actions.size(); ++i) {
        json += "{\"action_id\":\"" + m_actions[i].action_id + "\",";
        json += "\"sequence_id\":\"" + m_actions[i].sequence_id + "\",";
        // .index() reveals which variant type is currently held
        json += "\"type_index\":" + std::to_string(m_actions[i].payload.index()) + "}";
        if (i < m_actions.size() - 1) {
            json += ",";
        }
    }
    json += "]";
    return json;
}

void ExecutionCallStack::abort() {
    {
        std::scoped_lock lock(m_mutex);
        m_abort = true;
    }
    m_cv.notify_all();
}