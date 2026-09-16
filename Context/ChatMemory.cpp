#include "ChatMemory.h"

std::string ChatMemory::toMarkdown() const {
    std::ostringstream oss;

    if (!goals.empty()) {
        oss << "### User Goals & Progress:\n";
        for (const auto& g : goals) oss << "- " << g << "\n";
        oss << "\n";
    }

    if (!envFacts.empty()) {
        oss << "### Environment & User Facts:\n";
        for (const auto& f : envFacts) oss << "- " << f << "\n";
        oss << "\n";
    }

    if (!fileInsights.empty()) {
        oss << "### File Insights Cache:\n";
        for (const auto& [file, insight] : fileInsights) {
            oss << "- `" << file << "`: " << insight << "\n";
        }
        oss << "\n";
    }

    return oss.str();
}

bool ChatMemory::empty() const noexcept {
    return goals.empty() && envFacts.empty() && fileInsights.empty();
}

void ChatMemory::updateFileInsight(const std::string &path, const std::string &insight) {
    if (!path.empty() && !insight.empty()) {
        fileInsights[path] = insight;
    }
}

void ChatMemory::appendGoalDetail(const std::string &goal) {
    if (!goal.empty()) goals.push_back(goal);
}

void ChatMemory::addEnvFact(const std::string &fact) {
    if (!fact.empty()) envFacts.push_back(fact);
}