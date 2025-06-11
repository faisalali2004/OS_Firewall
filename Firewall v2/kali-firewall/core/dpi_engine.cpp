#include "dpi_engine.h"
#include <regex>
#include <mutex>
#include <algorithm>
#include <iostream>

DPIEngine::DPIEngine() {}

DPIEngine::~DPIEngine() {}

std::regex DPIEngine::make_regex(const std::string& pattern, bool case_insensitive) {
    try {
        return std::regex(pattern, case_insensitive ? std::regex::icase : std::regex::ECMAScript);
    } catch (const std::regex_error& e) {
        std::cerr << "[DPIEngine] Invalid regex pattern: " << pattern << " (" << e.what() << ")" << std::endl;
        // Fallback: match nothing
        return std::regex("$a");
    }
}

bool DPIEngine::addSignature(const std::string& name, const std::string& regex_str, DPIResult result, bool case_insensitive) {
    std::lock_guard<std::mutex> lock(mutex_);
    // Prevent duplicate signature names
    auto it = std::find_if(signatures.begin(), signatures.end(),
        [&](const Signature& sig) { return sig.name == name; });
    if (it != signatures.end()) {
        std::cerr << "[DPIEngine] Signature with name '" << name << "' already exists." << std::endl;
        return false;
    }
    Signature sig{name, make_regex(regex_str, case_insensitive), result, regex_str, case_insensitive};
    signatures.push_back(sig);
    return true;
}

bool DPIEngine::removeSignature(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto old_size = signatures.size();
    signatures.erase(
        std::remove_if(signatures.begin(), signatures.end(),
            [&](const Signature& sig) { return sig.name == name; }),
        signatures.end()
    );
    return signatures.size() < old_size;
}

std::vector<DPIEngine::SignatureInfo> DPIEngine::listSignatures() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<SignatureInfo> infos;
    for (const auto& sig : signatures) {
        infos.push_back({sig.name, sig.regex_str, sig.result, sig.case_insensitive});
    }
    return infos;
}

DPIResult DPIEngine::testPayload(const std::string& payload, std::string* matchedSig) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& sig : signatures) {
        try {
            if (std::regex_search(payload, sig.pattern)) {
                if (matchedSig) *matchedSig = sig.name;
                return sig.result;
            }
        } catch (const std::regex_error& e) {
            std::cerr << "[DPIEngine] Regex error for signature '" << sig.name << "': " << e.what() << std::endl;
            continue;
        }
    }
    if (matchedSig) *matchedSig = "";
    return DPIResult::Allow;
}

DPIResult DPIEngine::inspect(const uint8_t* data, size_t len, std::string& matchedSig) {
    std::string payload(reinterpret_cast<const char*>(data), len);
    return testPayload(payload, &matchedSig);
}

// --- REQUIRED FOR PACKET CAPTURE INTEGRATION ---
bool DPIEngine::shouldBlock(const std::string& src_ip,
                            const std::string& dst_ip,
                            int src_port,
                            int dst_port,
                            const std::string& protocol,
                            const unsigned char* payload,
                            int payload_len)
{
    if (!payload || payload_len <= 0)
        return false;

    std::string matchedSig;
    DPIResult result = inspect(payload, payload_len, matchedSig);

    // Block if result is Block, or you can extend to block on other types if desired
    return result == DPIResult::Block;
}