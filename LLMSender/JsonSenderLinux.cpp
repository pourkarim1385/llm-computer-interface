#include "JsonSenderLinux.hpp"

using json = nlohmann::json;


size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append((char*)contents, totalSize);
    return totalSize;
}

std::string SendDataToLLM(const std::string& apiKey, const std::string& endpoint, const std::string& user_prompt,
    const std::string sysData, const std::string image = "", const std::string file = ""){
    CURL* curl;
    CURLcode res;
    std::string response_string;

    curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize cURL." << std::endl;
        return "";
    }

    json user_content = json::array();

    user_content.push_back({
        {"type", "text"},
        {"text", user_prompt}
    });

    if (!file.empty()) {
        user_content.push_back({
            {"type", "text"},
            {"text", "\n\n--- Attached File Content ---\n" + file}
        });
    }


    if (!image.empty()) {
        user_content.push_back({
            {"type", "image_url"},
            {"image_url", {
                {"url", "data:image/jpeg;base64," + image}
            }}
        });
    }

    json payload = {
        {"model", "gpt-4o"},
        {"messages", {
            {{"role", "system"}, {"content", sysData}},
            {{"role", "user"}, {"content", user_prompt}}
        }},
        {"temperature", 0.7}
    };
    std::string json_payload = payload.dump();

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string auth_header = "Authorization: Bearer " + apiKey;
    headers = curl_slist_append(headers, auth_header.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return response_string;
}

