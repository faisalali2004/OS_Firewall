#pragma once

#include <string>
#include <vector>
#include <regex>
#include <mutex>

/**
 * @brief DPIResult enumerates recognized protocol types.
 */
enum class DPIResult {
    NONE,
    UNKNOWN,
    HTTP,
    DNS,
    TLS,
    SSH,
    FTP,
    SMTP,
    QUIC
};

/**
 * @brief DPIEngine provides thread-safe, extensible deep packet inspection.
 *        Supports binary-safe regex matching and custom signature management.
 */
class DPIEngine {
public:
    DPIEngine();

    /**
     * @brief Add a new protocol signature.
     * @param name Human-readable name for the signature.
     * @param regex_str Regex pattern (binary-safe, ECMAScript).
     * @param result DPIResult to return if matched.
     * @param case_insensitive If true, match case-insensitively.
     */
    void addSignature(const std::string& name, const std::string& regex_str, DPIResult result, bool case_insensitive = false);

    /**
     * @brief Remove a signature by name.
     */
    void removeSignature(const std::string& name);

    /**
     * @brief List all signature names.
     */
    std::vector<std::string> listSignatures() const;

    /**
     * @brief Inspect a packet payload and return the detected protocol.
     * @param payload Pointer to packet data.
     * @param len Length of packet data.
     * @param matched_info Set to the name of the matched signature or reason for no match.
     * @return DPIResult enum value.
     */
    DPIResult inspect(const uint8_t* payload, size_t len, std::string& matched_info);

private:
    struct Signature {
        std::string name;
        std::regex pattern;
        DPIResult result;
    };

    std::vector<Signature> signatures;
    mutable std::mutex mutex_;
};

    // Helper to create regex with options
    static std::regex make_regex(const std::string& pattern, bool case_insensitive = false);
};  