#include "JsonSender.hpp"
#include <string>
#include "curl/curl.h"

std::string JsonSender::IntegratedDetail(const std::vector<std::string>& detail) {
    std::string integratedDetail;
    integratedDetail.reserve(detail.size() * 100);
    for (const auto& str : detail) {
        integratedDetail += str;
    }
    return integratedDetail;
}

size_t JsonSender::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    if (userp) userp->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

std::string JsonSender::SendDataToLLM(
    const std::string& apiKey,
    const std::string& endpoint,
    const std::string& user_prompt,
    const std::string& sysData,
    const json& tools,
    const std::string& image,
    const std::string& file,
    const std::string& model,
    double temperature
) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "[cURL Error] Failed to initialize cURL." << std::endl;
        return "";
    }

    std::string response_string;

    json user_content = json::array();

    if (!user_prompt.empty()) {
        user_content.push_back({{"type", "text"}, {"text", user_prompt}});
    }
    if (!file.empty()) {
        user_content.push_back({
            {"type", "text"},
            {"text", "\n\n--- Attached File Content ---\n" + file}
        });
    }
    if (!image.empty()) {
        user_content.push_back({
            {"type", "image_url"},
            {"image_url", {{"url", "data:image/jpeg;base64," + image}}}
        });
    }

    json messages = json::array();
    if (!sysData.empty()) {
        messages.push_back({{"role", "system"}, {"content", sysData}});
    }
    messages.push_back({{"role", "user"}, {"content", user_content}});

    json payload = {
        {"model",       model},
        {"messages",    messages},
        {"temperature", temperature}
    };

    if (!tools.is_null() && !tools.empty()) {
        payload["tools"]       = tools;
        payload["tool_choice"] = "auto";
    }

    std::string json_payload = payload.dump();

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string auth_header = "Authorization: Bearer " + apiKey;
    headers = curl_slist_append(headers, auth_header.c_str());

    curl_easy_setopt(curl, CURLOPT_URL,           endpoint.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,    headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,    json_payload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,     &response_string);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,       60L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "[cURL Error] " << curl_easy_strerror(res) << std::endl;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return response_string;
}
