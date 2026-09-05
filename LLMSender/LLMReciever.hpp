#pragma once

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <iostream>
#include <map>
#include <vector>
#include "Actions.h"
#include "ExecutionCallStack.h"

using namespace std;
using json = nlohmann::json;

class LLMReciever
{
private:
    int sequenceId = 1;

    Actions::MouseButton parseMouseButton(const std::string& btn);
    Actions::Action parseAction(const std::string& tool, const json& args);
    LLMReciever() = default;
    LLMReciever(const LLMReciever&) = delete;
    LLMReciever& operator=(const LLMReciever&) = delete;
    LLMReciever(LLMReciever&&) = delete;
    LLMReciever& operator=(LLMReciever&&) = delete;
public:
    
    static LLMReciever& getInstance() {
        static LLMReciever instance;
        return instance;
    }

    //TODO: after fully implementing parse method we can delete this method
    std::vector<ActionItem> parseLLMResponse(
        const std::string& rawJson,
        std::vector<ActionItem>& actionItems,
        Plan& userPlan);
    json BuildToolsSchema();   
    void parse(const std::string& rawJson, ExecutionCallStack& callStack);
};