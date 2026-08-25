#pragma once

#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
std::string SendDataToLLM(const std::string& apiKey, const std::string& endpoint, const std::string& user_prompt,
    const std::string sysData, const std::string image, const std::string file);
