#pragma once

#include "SearchTypes.h"
#include "SearchExceptions.h"
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace WebSearch {
    class CreditManager {
    private:
        std::atomic<int64_t> max_limit_{UNLIMITED_CREDITS};
        std::atomic<int64_t> used_count_{0};

    public:
        explicit CreditManager(int64_t limit = UNLIMITED_CREDITS)
                : max_limit_(limit), used_count_(0) {}

        void setLimit(int64_t limit) {
            max_limit_.store(limit, std::memory_order_relaxed);
        }

        void addCredits(int64_t amount) {
            if (max_limit_.load(std::memory_order_relaxed) != UNLIMITED_CREDITS) {
                max_limit_.fetch_add(amount, std::memory_order_relaxed);
            }
        }

        void resetUsage() {
            used_count_.store(0, std::memory_order_relaxed);
        }

        bool hasCredits(int64_t cost = 1) const {
            int64_t limit = max_limit_.load(std::memory_order_relaxed);
            if (limit == UNLIMITED_CREDITS) return true;
            return (used_count_.load(std::memory_order_relaxed) + cost) <= limit;
        }

        bool tryConsume(int64_t cost = 1) {
            int64_t limit = max_limit_.load(std::memory_order_relaxed);
            if (limit == UNLIMITED_CREDITS) {
                used_count_.fetch_add(cost, std::memory_order_relaxed);
                return true;
            }

            int64_t current = used_count_.load(std::memory_order_relaxed);
            while (current + cost <= limit) {
                if (used_count_.compare_exchange_weak(current, current + cost,
                                                      std::memory_order_release,
                                                      std::memory_order_relaxed)) {
                    return true;
                }
            }
            return false;
        }

        void refund(int64_t cost = 1) {
            used_count_.fetch_sub(cost, std::memory_order_relaxed);
        }

        int64_t getRemainingCredits() const {
            int64_t limit = max_limit_.load(std::memory_order_relaxed);
            if (limit == UNLIMITED_CREDITS) return UNLIMITED_CREDITS;
            int64_t used = used_count_.load(std::memory_order_relaxed);
            return (limit >= used) ? (limit - used) : 0;
        }

        int64_t getUsedCredits() const { return used_count_.load(std::memory_order_relaxed); }
        int64_t getMaxLimit() const { return max_limit_.load(std::memory_order_relaxed); }
    };

    class SearchProvider {
    protected:
        std::string name_;
        CreditManager credit_manager_;

    public:
        explicit SearchProvider(std::string name, int64_t initial_limit = UNLIMITED_CREDITS)
                : name_(std::move(name)), credit_manager_(initial_limit) {}

        virtual ~SearchProvider() = default;

        const std::string& getName() const noexcept { return name_; }

        void setCreditLimit(int64_t limit) { credit_manager_.setLimit(limit); }
        void addCredits(int64_t amount) { credit_manager_.addCredits(amount); }
        void resetUsage() { credit_manager_.resetUsage(); }
        int64_t getRemainingCredits() const { return credit_manager_.getRemainingCredits(); }
        int64_t getUsedCredits() const { return credit_manager_.getUsedCredits(); }
        int64_t getMaxLimit() const { return credit_manager_.getMaxLimit(); }
        bool hasCredits(int64_t cost = 1) const { return credit_manager_.hasCredits(cost); }

        // Throws CreditExhaustedException, NetworkException, ParseException
        virtual SearchResponse search(const SearchRequest& request) = 0;
    };

    class TavilySearchProvider : public SearchProvider {
    private:
        std::string api_key_;

    public:
        explicit TavilySearchProvider(std::string api_key, int64_t initial_limit = 1000);
        SearchResponse search(const SearchRequest& request) override;
    };

    class DuckDuckGoSearchProvider : public SearchProvider {
    private:
        std::string sidecar_url_;

    public:
        explicit DuckDuckGoSearchProvider(std::string sidecar_url = "http://127.0.0.1:8000/search",
                                          int64_t initial_limit = UNLIMITED_CREDITS);
        SearchResponse search(const SearchRequest& request) override;
    };

}