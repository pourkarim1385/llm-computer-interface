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

std::string JsonSender::sendDataToLLM(
    const std::string& apiKey,
    const std::string& endpoint,
    const std::string& user_prompt,
    const std::string& sysData,
    const json& tools,
    std::shared_ptr<WorldState> worldState,
    const std::string& model,
    double temperature
) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "[cURL Error] Failed to initialize cURL." << std::endl;
        return "";
    }

    std::string response_string;

    std::string combined_text = user_prompt;

    if (worldState) {
        const auto& footnotes = worldState->getFootnotes();
        if (!footnotes.empty()) {
            std::string integratedFootnotes;
            for (const auto& note : footnotes) {
                integratedFootnotes += note + "\n\n";
            }
            combined_text += "\n\n--- Environment Context & Footnotes ---\n" + integratedFootnotes;
        }
    }

    if (!tools.is_null() && !tools.empty()) {
        combined_text += "\n\n### Available Tools & Parameter Schemas:\n"
                         "You must plan actions using only the tools defined below. "
                         "Populate the 'tool' and 'arguments' fields of your 'steps' schema according to these specifications:\n"
                         + tools.dump(2);
    }

    std::vector<MediaPayload> Media;
    if (worldState) {
        Media = worldState->getUploadList();
    }

    bool has_images = false;
    for (const auto& obj : Media) {
        if (obj.mimeType != ".pdf" && obj.mimeType != ".mp3" && obj.mimeType != ".wav") {
            has_images = true;
            break;
        }
    }

    json user_message_content;

    if (!has_images) {
        for (const auto& obj : Media) {
            combined_text += "\n\n--- Attached File Content ---\n" + obj.base64;
        }
        user_message_content = combined_text.empty() ? " " : combined_text;
    } else {
        json content_array = json::array();
        if (!combined_text.empty()) {
            content_array.push_back({{"type", "text"}, {"text", combined_text}});
        }
        for (const auto& obj : Media) {
            if (obj.mimeType == ".pdf" || obj.mimeType == ".mp3" || obj.mimeType == ".wav") {
                content_array.push_back({
                    {"type", "text"},
                    {"text", "\n\n--- Attached File Content ---\n" + obj.base64}
                });
            } else {
                std::string clean_mime = obj.mimeType;
                if (!clean_mime.empty() && clean_mime[0] == '.') {
                    clean_mime = clean_mime.substr(1);
                }
                content_array.push_back({
                    {"type", "image_url"},
                    {"image_url", {{"url", "data:image/" + clean_mime + ";base64," + obj.base64}}}
                });
            }
        }
        user_message_content = content_array;
    }

    json messages = json::array();
    if (!sysData.empty()) {
        messages.push_back({{"role", "system"}, {"content", sysData}});
    }
    messages.push_back({{"role", "user"}, {"content", user_message_content}});

    // Build payload
    json payload = {
        {"model",           model},
        {"messages",        messages},
        {"temperature",     temperature},
        {"response_format", {{"type", "json_object"}}}
    };

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
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,        180L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "[cURL Error] " << curl_easy_strerror(res) << std::endl;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return response_string;
}

std::string JsonSender::sendDataToLLM(
    const std::string& apiKey,
    const std::string& endpoint,
    const std::string& user_prompt,
    const std::string& sysData,
    const std::string& model,
    double temperature
) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "[cURL Error] Failed to initialize cURL." << std::endl;
        return "";
    }

    std::string response_string;

    json messages = json::array();
    if (!sysData.empty()) {
        messages.push_back({{"role", "system"}, {"content", sysData}});
    }
    if (!user_prompt.empty()) {
        messages.push_back({{"role", "user"}, {"content", user_prompt}});
    }

    json payload = {
        {"model",       model},
        {"messages",    messages},
        {"temperature", temperature}
    };

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