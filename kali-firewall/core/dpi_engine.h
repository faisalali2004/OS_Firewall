#pragma once

#include <string>
#include <vector>
#include <regex>
#include <mutex>

enum class DPIResult {
    Allow,
    Block
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

    bool addSignature(const std::string& name, const std::string& regex_str, DPIResult result, bool case_insensitive = false);
    bool removeSignature(const std::string& name);
    std::vector<SignatureInfo> listSignatures();

    // Main DPI inspection interface
    DPIResult inspect(const uint8_t* data, size_t len, std::string& matchedSig);

    // For packet_capture.cpp integration
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
        std::regex pattern;
        DPIResult result;
        std::string regex_str;
        bool case_insensitive;
    };

    std::regex make_regex(const std::string& pattern, bool case_insensitive);
    DPIResult testPayload(const std::string& payload, std::string* matchedSig);

    std::vector<Signature> signatures;
    std::mutex mutex_;
};