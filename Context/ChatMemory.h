#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>

class ChatMemory {
public:
    ChatMemory() = default;

    [[nodiscard]] const std::vector<std::string>& getGoals() const noexcept { return goals; }
    [[nodiscard]] const std::vector<std::string>& getEnvFacts() const noexcept { return envFacts; }
    [[nodiscard]] const std::unordered_map<std::string, std::string>& getFileInsights() const noexcept { return fileInsights; }

    void setGoals(std::vector<std::string> g) { goals = std::move(g); }
    void setEnvFacts(std::vector<std::string> f) { envFacts = std::move(f); }
    void setFileInsights(std::unordered_map<std::string, std::string> fi) { fileInsights = std::move(fi); }

    void appendGoalDetail(const std::string& goal);
    void addEnvFact(const std::string& fact);
    void updateFileInsight(const std::string& path, const std::string& insight);
    [[nodiscard]] bool shouldCompressGoals() const noexcept {return goals.size() >= maxGoals;}
    [[nodiscard]] bool shouldCompressFacts() const noexcept {return envFacts.size() >= maxFacts;}
    [[nodiscard]] bool shouldCompressFiles() const noexcept {return fileInsights.size() >= maxFiles;}
    void setCompressedGoals(std::vector<std::string> newGoals) {goals = std::move(newGoals);}
    void setCompressedFacts(std::vector<std::string> newFacts) {envFacts = std::move(newFacts);}

    [[nodiscard]] std::string toMarkdown() const;
    [[nodiscard]] bool empty() const noexcept;

private:
    std::vector<std::string> goals;
    std::vector<std::string> envFacts;
    std::unordered_map<std::string, std::string> fileInsights;

    static constexpr size_t maxGoals = 10;
    static constexpr size_t maxFacts = 20;
    static constexpr size_t maxFiles = 10;
};