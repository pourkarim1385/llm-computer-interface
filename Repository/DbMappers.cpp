#include "DbMappers.h"

// Implementations are placed in the agent::chat namespace
namespace agent::chat {
    void to_json(json& j, const Step& step) {
        j = json{{"title", step.title}, {"content", step.content}, {"isDone", step.isDone}};
    }

    void from_json(const json& j, Step& step) {
        j.at("title").get_to(step.title);
        j.at("content").get_to(step.content);
        j.at("isDone").get_to(step.isDone);
    }

    void to_json(json& j, const Plan& plan) {
        j = json{{"name", plan.name}, {"description", plan.description}, {"steps", plan.steps}};
    }

    void from_json(const json& j, Plan& plan) {
        j.at("name").get_to(plan.name);
        j.at("description").get_to(plan.description);
        j.at("steps").get_to(plan.steps);
    }
}

namespace agent::storage {
    using json = nlohmann::json;

    std::string Mapper::serializePlan(const chat::Plan& plan) {
        json j = plan;
        return j.dump();
    }

    chat::Plan Mapper::deserializePlan(const std::string& serializedPlan) {
        if (serializedPlan.empty()) return {};
        return json::parse(serializedPlan).get<chat::Plan>();
    }

    std::string Mapper::serializeHeaders(const std::unordered_map<std::string, std::string>& headers) {
        json j = headers;
        return j.dump();
    }

    std::unordered_map<std::string, std::string> Mapper::deserializeHeaders(const std::string& json_str) {
        if (json_str.empty()) return {};
        return json::parse(json_str).get<std::unordered_map<std::string, std::string>>();
    }
}