#pragma once

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <iostream>
#include <map>
#include <vector>
#include "Actions.hpp"
#include "ExecutionCallStack.hpp"


using namespace std;

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
public:

    Actions::Action parseAction(const std::string& tool, const json& args);
    std::vector<ActionItem> parseLLMResponse(
        const std::string& raw_json,
        std::vector<ActionItem>& actionItems,
        std::vector<Description>& descriptions);
    LLMReciever() = default;
    ~LLMReciever() = default;
};
