#pragma once

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <iostream>
#include <map>
#include <vector>
#include <string.h>
#include "Actions.hpp"
#include "ExecutionCallStack.hpp"


using namespace std;
using json = nlohmann::json;


struct Description
{
    string sequenceId;
    string partId;
    string description;
};


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
    
    std::vector<ActionItem> parseLLMResponse(
        const std::string& rawJson,
        std::vector<ActionItem>& actionItems,
        std::vector<Description>& descriptions);

};
