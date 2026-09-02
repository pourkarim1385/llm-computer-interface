#include "Security/SecretVault.h"
#include <stdexcept>
#include <iomanip>
#include <sstream>

#ifdef WIN32
#include <windows.h>
#include <wincrypt.h>
#endif

namespace agent::security {

    std::string SecretVault::encrypt(const std::string& plaintext) {
        if (plaintext.empty()) return "";

#ifdef WIN32
        DATA_BLOB dataIn;
        dataIn.pbData = reinterpret_cast<BYTE*>(const_cast<char*>(plaintext.data()));
        dataIn.cbData = static_cast<DWORD>(plaintext.size());

        DATA_BLOB dataOut;
        if (CryptProtectData(&dataIn, nullptr, nullptr, nullptr, nullptr, 0, &dataOut)) {
            std::string hexResult = bytesToHex(dataOut.pbData, dataOut.cbData);
            LocalFree(dataOut.pbData);
            return hexResult;
        }
        throw std::runtime_error("DPAPI Encryption failed");
#else
        // TODO: Implement Libsecret for Linux
        return plaintext;
#endif
    }

    std::string SecretVault::decrypt(const std::string& ciphertext) {
        if (ciphertext.empty()) return "";

#ifdef WIN32
        std::string rawBytes = hexToBytes(ciphertext);
        DATA_BLOB dataIn;
        dataIn.pbData = reinterpret_cast<BYTE*>(const_cast<char*>(rawBytes.data()));
        dataIn.cbData = static_cast<DWORD>(rawBytes.size());

        DATA_BLOB dataOut;
        if (CryptUnprotectData(&dataIn, nullptr, nullptr, nullptr, nullptr, 0, &dataOut)) {
            std::string plaintext(reinterpret_cast<char*>(dataOut.pbData), dataOut.cbData);
            LocalFree(dataOut.pbData);
            return plaintext;
        }
        // If decryption fails (e.g., wrong user), return empty or throw
        return "";
#else
        return ciphertext;
#endif
    }

    // --- Helper: Hex Encoding for Safe JSON/DB Storage ---
    std::string SecretVault::bytesToHex(const unsigned char* data, size_t length) {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (size_t i = 0; i < length; ++i)
            oss << std::setw(2) << static_cast<int>(data[i]);
        return oss.str();
    }

    std::string SecretVault::hexToBytes(const std::string& hex) {
        std::string bytes;
        for (size_t i = 0; i < hex.length(); i += 2) {
            std::string byteString = hex.substr(i, 2);
            char byte = (char)strtol(byteString.c_str(), nullptr, 16);
            bytes.push_back(byte);
        }
        return bytes;
    }
}