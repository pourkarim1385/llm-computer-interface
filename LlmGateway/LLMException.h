#pragma once

#include <stdexcept>
#include <string>

enum class LLMErrorType {
    Validation,
    ParseAction,
    ProtocolViolation,
    ApiError
};

inline std::string errorTypeToString(LLMErrorType type) {
    switch (type) {
        case LLMErrorType::Validation:
            return "Validation";
        case LLMErrorType::ParseAction:
            return "ParseAction";
        case LLMErrorType::ProtocolViolation:
            return "ProtocolViolation";
        case LLMErrorType::ApiError:
            return "ApiError";
        default:
            return "Unknown";
    }
}

class LLMException : public std::runtime_error {
private:
    LLMErrorType m_errorType;
    std::string m_reason;

    static std::string formatMessage(LLMErrorType type, const std::string& reason) {
        return "[" + errorTypeToString(type) + "] " + reason;
    }

public:
    LLMException(LLMErrorType type, const std::string& reason)
        : std::runtime_error(formatMessage(type, reason)),
          m_errorType(type),
          m_reason(reason) {}

    LLMErrorType getErrorType() const noexcept {
        return m_errorType;
    }

    const std::string& getReason() const noexcept {
        return m_reason;
    }

    std::string getErrorTypeString() const noexcept {
        return errorTypeToString(m_errorType);
    }
};