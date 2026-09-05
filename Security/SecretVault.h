#pragma once
#include <string>

namespace agent::security {

    class SecretVault {
    public:
        // Takes a raw API key and returns an encrypted, hex-encoded string safe for SQLite/JSON
        static std::string encrypt(const std::string& plaintext);

        // Takes the hex-encoded ciphertext and decrypts it back to the usable API key
        static std::string decrypt(const std::string& ciphertext);

    private:
        static std::string bytesToHex(const unsigned char* data, size_t length);
        static std::string hexToBytes(const std::string& hex);
    };

} // namespace agent::security