#pragma once

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <iostream>
#include <map>
#include <vector>
#include "Actuation/Actions.h"
#include "Actuation/ExecutionCallStack.h"

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

    void parse(const std::string& rawJson, ExecutionCallStack& callStack, Plan& userPlan, std::string& messageToUser);
};