#include "SecretVault.h"
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#include <dpapi.h>
#pragma comment(lib, "crypt32.lib")

namespace agent::storage {
    std::string SecretVault::encrypt(const std::string& plaintext) {
        if (plaintext.empty()) return "";
        DATA_BLOB in, out;
        in.pbData = reinterpret_cast<BYTE*>(const_cast<char*>(plaintext.data()));
        in.cbData = static_cast<DWORD>(plaintext.size());

        if (CryptProtectData(&in, L"AgentSecret", nullptr, nullptr, nullptr, 0, &out)) {
            std::string result(reinterpret_cast<char*>(out.pbData), out.cbData);
            LocalFree(out.pbData);
            return result;
        }
        throw std::runtime_error("DPAPI Encryption failed");
    }

    std::string SecretVault::decrypt(const std::string& ciphertext) {
        if (ciphertext.empty()) return "";
        DATA_BLOB in, out;
        in.pbData = reinterpret_cast<BYTE*>(const_cast<char*>(ciphertext.data()));
        in.cbData = static_cast<DWORD>(ciphertext.size());

        if (CryptUnprotectData(&in, nullptr, nullptr, nullptr, nullptr, 0, &out)) {
            std::string result(reinterpret_cast<char*>(out.pbData), out.cbData);
            LocalFree(out.pbData);
            return result;
        }
        throw std::runtime_error("DPAPI Decryption failed");
    }
}
#else
namespace agent::storage {
    std::string SecretVault::encrypt(const std::string& plaintext) {
        return plaintext; //TODO: Implement libsecret or XOR obfuscation here
    }
    std::string SecretVault::decrypt(const std::string& ciphertext) {
        return ciphertext; //TODO: Implement libsecret or XOR obfuscation here
    }
}
#endif