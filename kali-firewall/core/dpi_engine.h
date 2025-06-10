#pragma once
#include <string>
#include <vector>
#include <regex>
#include <mutex>

enum class DPIResult { Allow, Block };

class DPIEngine {
public:
    DPIEngine();
    ~DPIEngine();

    struct SignatureInfo {
        std::string name;
        std::string regex_str;
        DPIResult result;
        bool case_insensitive;
    };

    // Returns true if added, false if duplicate
    bool addSignature(const std::string& name, const std::string& regex_str, DPIResult result, bool case_insensitive = false);
    // Returns true if removed, false if not found
    bool removeSignature(const std::string& name);
    // List all signatures with details
    std::vector<SignatureInfo> listSignatures();
    // Test payload, returns result and optionally matched signature name
    DPIResult testPayload(const std::string& payload, std::string* matchedSig = nullptr);

    static std::regex make_regex(const std::string& pattern, bool case_insensitive = false);

private:
    struct Signature {
        std::string name;
        std::regex pattern;
        DPIResult result;
        std::string regex_str;
        bool case_insensitive;
    };
    std::vector<Signature> signatures;
    std::mutex mutex_;
};