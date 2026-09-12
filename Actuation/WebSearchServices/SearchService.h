#pragma once

#include "SearchTypes.h"
#include "SearchExceptions.h"
#include <string>
#include <vector>
#include <memory>

namespace WebSearch {
    class SearchService {
    public:
        explicit SearchService(const SearchConfig& config);

        ~SearchService();
        SearchService(SearchService&&) noexcept;
        SearchService& operator=(SearchService&&) noexcept;

        SearchService(const SearchService&) = delete;
        SearchService& operator=(const SearchService&) = delete;

        SearchResponse search(const SearchRequest& request);
        SearchResponse search(const std::string& query, int max_results = 5);

        void resetUsage();
        static void resetActiveUsage();

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl_;
    };
}