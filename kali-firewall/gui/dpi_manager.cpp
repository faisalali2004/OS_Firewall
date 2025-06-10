#include "dpi_engine.h"
#include <cstring>
#include <iostream>
#include <stdexcept>

DPIEngine::DPIEngine() {
    try {
        // Add default protocol signatures (regex patterns)
        signatures.push_back({"HTTP", std::regex(R"(^GET |^POST |^HTTP/1\.)", std::regex::optimize), DPIResult::HTTP});
        signatures.push_back({"DNS", std::regex(R"(^.{2}\x01\x00)", std::regex::optimize), DPIResult::DNS});
        signatures.push_back({"TLS", std::regex(R"(^\x16\x03)", std::regex::optimize), DPIResult::TLS});
        signatures.push_back({"SSH", std::regex(R"(^SSH-)", std::regex::optimize), DPIResult::SSH});
    } catch (const std::exception& e) {
        std::cerr << "[DPIEngine] Error initializing signatures: " << e.what() << std::endl;
    }
}

void DPIEngine::addSignature(const std::string& name, const std::string& regex_str, DPIResult result) {
    try {
        signatures.push_back({name, std::regex(regex_str, std::regex::optimize), result});
    } catch (const std::exception& e) {
        std::cerr << "[DPIEngine] Failed to add signature '" << name << "': " << e.what() << std::endl;
    }
}

void DPIEngine::clearSignatures() {
    signatures.clear();
}

const std::vector<DPISignature>& DPIEngine::getSignatures() const {
    return signatures;
}

DPIResult DPIEngine::inspect(const uint8_t* payload, size_t len, std::string& matched_info) {
    if (!payload || len == 0) {
        matched_info = "Empty payload";
        return DPIResult::NONE;
    }

    // Convert binary payload to string safely
    std::string data(reinterpret_cast<const char*>(payload), len);

    for (const auto& sig : signatures) {
        try {
            if (std::regex_search(data, sig.pattern)) {
                matched_info = sig.name;
                return sig.result;
            }
        } catch (const std::exception& e) {
            std::cerr << "[DPIEngine] Regex error for signature '" << sig.name << "': " << e.what() << std::endl;
            continue;
        }
    }
    matched_info = "No match";
    return DPIResult::UNKNOWN;
}