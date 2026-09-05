#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace WebSearch {
    constexpr int64_t UNLIMITED_CREDITS = -1;

    struct SearchRequest {
        std::string query;
        int max_results = 5;
        std::string search_depth = "basic";
        bool include_raw_content = false;
        bool include_answer = false;
    };

    struct SearchResultItem {
        std::string title;
        std::string url;
        std::string content;
        std::string raw_content = "";
        float score = 0.0f;
    };

    struct SearchResponse {
        std::string provider_name;
        std::string query;
        std::string answer;
        std::vector<SearchResultItem> results;
        int64_t remaining_credits = UNLIMITED_CREDITS;

        std::string to_llm_context() const;
    };

    struct SearchConfig {
        std::string c_api_key;
        int64_t c_credit_limit = 1000;

        std::string ddg_sidecar_url = "http://127.0.0.1:8000/search";
        bool enable_ddg_fallback = true;
    };

}