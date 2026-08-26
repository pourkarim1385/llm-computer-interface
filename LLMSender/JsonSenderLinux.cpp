#include "JsonSenderLinux.hpp"

using json = nlohmann::json;


inline std::string IntegratedDetail(const std::vector<std::string>& detail) {
    std::string integratedDetail;
    integratedDetail.reserve(detail.size() * 100);
    for (const auto& str : detail) {
        integratedDetail += str;
    }
    return integratedDetail;
}

inline size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    if (userp) {
        userp->append(static_cast<char*>(contents), totalSize);
    }
    return totalSize;
}


// This part creates a json for the api in an attemp to give a list of tools for the llm.
// it can be customized.
inline json BuildToolsSchema() {
    json tools = json::array({
        {
            {"type", "function"},
            {"function", {
                {"name", "get_system_info"},
                {"description", "Returns current system information such as CPU usage, RAM, OS, and active window title."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", json::object()},
                    {"required", json::array()}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "run_shell_command"},
                {"description", "Executes a shell command on the local system and returns stdout/stderr."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"command", {
                            {"type", "string"},
                            {"description", "The shell command to execute (e.g., 'ls -la', 'df -h')"}
                        }}
                    }},
                    {"required", json::array({"command"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "get_weather"},
                {"description", "Get current weather information for a specific location."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"location", {
                            {"type", "string"},
                            {"description", "The city name, e.g. 'Tehran', 'New York'"}
                        }}
                    }},
                    {"required", json::array({"location"})}
                }}
            }}
        }
    });
    return tools;
}

/**
 * ارسال درخواست به API مدل‌های زبانی (سازگار با OpenAI Chat Completions API)
 * 
 * @param apiKey کلید احراز هویت API
 * @param endpoint آدرس endpoint (مثلاً https://api.openai.com/v1/chat/completions)
 * @param user_prompt متن پرامپت کاربر
 * @param sysData پرامپت سیستمی
 * @param tools لیست ابزارها/توابع در قالب JSON (اگر خالی باشد، ارسال نمی‌شود)
 * @param image تصویر به صورت base64 (اختیاری)
 * @param file محتوای متنی فایل پیوست (اختیاری)
 * @param model نام مدل (پیش‌فرض: gpt-4o)
 * @param temperature دمای تولید پاسخ (پیش‌فرض: 0.7)
 * @return پاسخ JSON از API به صورت string
 */
inline std::string SendDataToLLM(
    const std::string& apiKey,
    const std::string& endpoint,
    const std::string& user_prompt,
    const std::string& sysData,
    const json& tools = json::array(),
    const std::string& image = "",
    const std::string& file = "",
    const std::string& model = "gpt-4o",
    double temperature = 0.7
) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "[cURL Error] Failed to initialize cURL." << std::endl;
        return "";
    }

    std::string response_string;

    // Creating user base content.
    json user_content = json::array();

    if (!user_prompt.empty()) {
        user_content.push_back({
            {"type", "text"},
            {"text", user_prompt}
        });
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
            {"image_url", {
                {"url", "data:image/jpeg;base64," + image}
            }}
        });
    }

    // Creating messages
    json messages = json::array();

    if (!sysData.empty()) {
        messages.push_back({
            {"role", "system"},
            {"content", sysData}
        });
    }

    messages.push_back({
        {"role", "user"},
        {"content", user_content}  
    });

    // Main payload
    json payload = {
        {"model", model},
        {"messages", messages},
        {"temperature", temperature}
    };

    // Tools check
    if (!tools.is_null() && !tools.empty()) {
        payload["tools"] = tools;
        payload["tool_choice"] = "auto";
    }

    std::string json_payload = payload.dump();

    // header management
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string auth_header = "Authorization: Bearer " + apiKey;
    headers = curl_slist_append(headers, auth_header.c_str());

    // Option management
    curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);

    // Acting the operation
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "[cURL Error] " << curl_easy_strerror(res) << std::endl;
    }

    // Cleaning
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return response_string;
}
