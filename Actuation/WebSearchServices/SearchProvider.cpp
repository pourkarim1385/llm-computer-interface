#include "SearchProvider.h"
#include <cpr/cpr.h>
#include <sstream>

using json = nlohmann::json;

namespace WebSearch {
    TavilySearchProvider::TavilySearchProvider(std::string api_key, int64_t initial_limit)
            : SearchProvider("Tavily", initial_limit), api_key_(std::move(api_key)) {}

    SearchResponse TavilySearchProvider::search(const SearchRequest& request) {
        int64_t cost = (request.search_depth == "advanced") ? 2 : 1;

        // Check & Consume credits
        if (!credit_manager_.tryConsume(cost)) {
            throw CreditExhaustedException(name_, credit_manager_.getRemainingCredits());
        }

        json payload = {
                {"query", request.query},
                {"max_results", request.max_results},
                {"search_depth", request.search_depth},
                {"include_raw_content", request.include_raw_content},
                {"include_answer", request.include_answer}
        };

        cpr::Response r = cpr::Post(
                cpr::Url{"https://api.tavily.com/search"},
                cpr::Header{
                        {"Authorization", "Bearer " + api_key_},
                        {"Content-Type", "application/json"}
                },
                cpr::Body{payload.dump()},
                cpr::Timeout{5000}
        );

        if (r.status_code != 200) {
            credit_manager_.refund(cost);
            throw NetworkException(name_, r.status_code, r.error.message.empty() ? r.text : r.error.message);
        }

        SearchResponse response;
        response.provider_name = name_;
        response.query = request.query;

        try {
            json res_json = json::parse(r.text);
            if (res_json.contains("answer") && !res_json["answer"].is_null()) {
                response.answer = res_json["answer"].get<std::string>();
            }

            for (const auto& item : res_json["results"]) {
                SearchResultItem res_item;
                res_item.title = item.value("title", "");
                res_item.url = item.value("url", "");
                res_item.content = item.value("content", "");
                res_item.score = item.value("score", 0.0f);
                if (item.contains("raw_content") && !item["raw_content"].is_null()) {
                    res_item.raw_content = item["raw_content"].get<std::string>();
                }
                response.results.push_back(res_item);
            }
        } catch (const json::exception& e) {
            credit_manager_.refund(cost);
            throw ParseException(name_, e.what());
        }

        response.remaining_credits = credit_manager_.getRemainingCredits();
        return response;
    }

    DuckDuckGoSearchProvider::DuckDuckGoSearchProvider(std::string sidecar_url, int64_t initial_limit)
            : SearchProvider("DuckDuckGo", initial_limit), sidecar_url_(std::move(sidecar_url)) {}

    SearchResponse DuckDuckGoSearchProvider::search(const SearchRequest& request) {
        if (!credit_manager_.tryConsume(1)) {
            throw CreditExhaustedException(name_, credit_manager_.getRemainingCredits());
        }

        json payload = {
                {"query", request.query},
                {"max_results", request.max_results}
        };

        cpr::Response r = cpr::Post(
                cpr::Url{sidecar_url_},
                cpr::Header{{"Content-Type", "application/json"}},
                cpr::Body{payload.dump()},
                cpr::Timeout{7000}
        );

        if (r.status_code != 200) {
            credit_manager_.refund(1);
            throw NetworkException(name_, r.status_code, r.error.message.empty() ? r.text : r.error.message);
        }

        SearchResponse response;
        response.provider_name = name_;
        response.query = request.query;

        try {
            json res_json = json::parse(r.text);
            for (const auto& item : res_json["results"]) {
                SearchResultItem res_item;
                res_item.title = item.value("title", "");
                res_item.url = item.value("href", "");
                res_item.content = item.value("body", "");
                response.results.push_back(res_item);
            }
        } catch (const json::exception& e) {
            credit_manager_.refund(1);
            throw ParseException(name_, e.what());
        }

        response.remaining_credits = credit_manager_.getRemainingCredits();
        return response;
    }

}