#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <utility>
#include <cstdint>

namespace WebSearch {
    class SearchException : public std::runtime_error {
    public:
        explicit SearchException(const std::string& message)
                : std::runtime_error(message) {}
    };

    class CreditExhaustedException : public SearchException {
    private:
        std::string provider_name_;
        int64_t remaining_credits_;

    public:
        CreditExhaustedException(std::string provider_name, int64_t remaining)
                : SearchException("Credit quota exhausted for provider: " + provider_name),
                  provider_name_(std::move(provider_name)),
                  remaining_credits_(remaining) {}

        const std::string& getProviderName() const noexcept { return provider_name_; }
        int64_t getRemainingCredits() const noexcept { return remaining_credits_; }
    };

    class NetworkException : public SearchException {
    private:
        std::string provider_name_;
        long status_code_;

    public:
        NetworkException(std::string provider_name, long status_code, const std::string& details)
                : SearchException("Network error on [" + provider_name + "] (Status " +
                                  std::to_string(status_code) + "): " + details),
                  provider_name_(std::move(provider_name)),
                  status_code_(status_code) {}

        const std::string& getProviderName() const noexcept { return provider_name_; }
        long getStatusCode() const noexcept { return status_code_; }
    };

    class ParseException : public SearchException {
    private:
        std::string provider_name_;

    public:
        ParseException(std::string provider_name, const std::string& details)
                : SearchException("Failed to parse JSON response from [" + provider_name + "]: " + details),
                  provider_name_(std::move(provider_name)) {}

        const std::string& getProviderName() const noexcept { return provider_name_; }
    };

    class AllProvidersFailedException : public SearchException {
    private:
        std::vector<std::pair<std::string, std::string>> failures_; // (ProviderName, Reason)

    public:
        explicit AllProvidersFailedException(std::vector<std::pair<std::string, std::string>> failures)
                : SearchException("All search providers failed to execute the query."),
                  failures_(std::move(failures)) {}

        const std::vector<std::pair<std::string, std::string>>& getFailures() const noexcept {
            return failures_;
        }

        std::string getDetailedReport() const {
            std::string report = "AllProvidersFailedException Summary:\n";
            for (const auto& [provider, reason] : failures_) {
                report += "  - Provider [" + provider + "] failed: " + reason + "\n";
            }
            return report;
        }
    };

}