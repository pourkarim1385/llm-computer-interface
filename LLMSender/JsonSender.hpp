#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "Observation/Models/WorldState.h"

using json = nlohmann::json; 

// The json is based on the openai api and similars.
class JsonSender
{
private:
    std::string IntegratedDetail(const std::vector<std::string>& detail);
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
public:

    JsonSender() = default;
    ~JsonSender() = default;
    std::string SendDataToLLM(
        const std::string& apiKey,
        const std::string& endpoint,
        const std::string& user_prompt,
        const std::string& sysData,
        const json& tools,
        WorldState worldState,
        const std::string& model,
        double temperature
    );
};
