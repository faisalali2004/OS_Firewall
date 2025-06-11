#include "dpi_engine.h"
#include <algorithm>
#include <cstring>
#include <iostream> // For error logging (replace with your logger if needed)

DPIEngine::DPIEngine() = default;
DPIEngine::~DPIEngine() = default;

std::regex DPIEngine::make_regex(const std::string& pattern, bool case_insensitive) {
    return std::regex(pattern, case_insensitive ? std::regex::icase : std::regex::ECMAScript);
}

bool DPIEngine::addSignature(const std::string& name, const std::string& regex_str, DPIResult result, bool case_insensitive) {
    std::lock_guard<std::mutex> lock(mutex_);
    // Prevent duplicate names
    auto it = std::find_if(signatures.begin(), signatures.end(),
        [&](const Signature& s) { return s.name == name; });
    if (it != signatures.end()) return false;

    try {
        Signature sig;
        sig.name = name;
        sig.regex_str = regex_str;
        sig.result = result;
        sig.case_insensitive = case_insensitive;
        sig.pattern = make_regex(regex_str, case_insensitive);
        signatures.push_back(std::move(sig));
        return true;
    } catch (const std::regex_error& e) {
        std::cerr << "DPIEngine: Invalid regex for signature '" << name << "': " << e.what() << std::endl;
        return false;
    }
}

bool DPIEngine::removeSignature(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::remove_if(signatures.begin(), signatures.end(),
        [&](const Signature& s) { return s.name == name; });
    if (it == signatures.end()) return false;
    signatures.erase(it, signatures.end());
    return true;
}

std::vector<DPIEngine::SignatureInfo> DPIEngine::listSignatures() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<SignatureInfo> infos;
    for (const auto& sig : signatures) {
        infos.push_back(SignatureInfo{sig.name, sig.regex_str, sig.result, sig.case_insensitive});
    }
    return infos;
}

DPIResult DPIEngine::inspect(const uint8_t* data, size_t len, std::string& matchedSig) {
    std::string payload(reinterpret_cast<const char*>(data), len);
    return testPayload(payload, &matchedSig);
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
            std::cerr << "DPIEngine: Regex error in signature '" << sig.name << "': " << e.what() << std::endl;
            continue;
        }
    }
    if (matchedSig) *matchedSig = "";
    return DPIResult::UNKNOWN;
}

bool DPIEngine::shouldBlock(const std::string& src_ip,
                            const std::string& dst_ip,
                            int src_port,
                            int dst_port,
                            const std::string& protocol,
                            const unsigned char* payload,
                            int payload_len) {
    (void)src_ip; (void)dst_ip; (void)src_port; (void)dst_port; (void)protocol; // suppress unused warnings
    std::string matched;
    DPIResult res = inspect(payload, payload_len, matched);
    return res == DPIResult::Block;
}