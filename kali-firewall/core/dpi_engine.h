#pragma once

#include <string>
#include <vector>
#include <regex>
#include <mutex>

enum class DPIResult {
    Allow,
    Block,
    HTTP,
    DNS,
    TLS,
    SSH,
    FTP,
    SMTP,
    QUIC,
    NONE,
    UNKNOWN
};

class DPIEngine {
public:
    struct SignatureInfo {
        std::string name;
        std::string regex_str;
        DPIResult result;
        bool case_insensitive;
    };

    DPIEngine();
    ~DPIEngine();

    // Add a new DPI signature. Returns false if name already exists or regex is invalid.
    bool addSignature(const std::string& name, const std::string& regex_str, DPIResult result, bool case_insensitive);

    // Remove a signature by name. Returns true if removed.
    bool removeSignature(const std::string& name);

    // List all signatures.
    std::vector<SignatureInfo> listSignatures();

    // Inspect a payload and return the DPIResult. matchedSig will be set to the matching signature name.
    DPIResult inspect(const uint8_t* data, size_t len, std::string& matchedSig);

    // Test a payload string directly (for GUI testing).
    DPIResult testPayload(const std::string& payload, std::string* matchedSig = nullptr);

    // Example: Should this packet be blocked?
    bool shouldBlock(const std::string& src_ip,
                    const std::string& dst_ip,
                    int src_port,
                    int dst_port,
                    const std::string& protocol,
                    const unsigned char* payload,
                    int payload_len);

private:
    struct Signature {
        std::string name;
        std::string regex_str;
        DPIResult result;
        bool case_insensitive;
        std::regex pattern;
    };

    std::vector<Signature> signatures;
    mutable std::mutex mutex_;

    static std::regex make_regex(const std::string& pattern, bool case_insensitive);
};