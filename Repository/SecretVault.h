#pragma once
#include <string>

namespace agent::storage {
    class SecretVault {
    public:
        static std::string encrypt(const std::string& plaintext);
        static std::string decrypt(const std::string& ciphertext);
    };
}