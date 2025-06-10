#include "dpi_engine.h"
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <mutex>
#include <algorithm>

namespace {
    std::regex make_regex(const std::string& pattern, bool case_insensitive = false) {
        return std::regex(
            pattern,
            std::regex::optimize | (case_insensitive ? std::regex::icase : std::regex::ECMAScript)
        );
    }
}

DPIEngine::DPIEngine() {
    try {
        // Add default protocol signatures (regex, optimized)
        addSignature("HTTP", R"((?:^|\r\n)(GET |POST |HEAD |HTTP/1\.))", DPIResult::HTTP, true);
        addSignature("DNS", R"(^.{2}\x01\x00)", DPIResult::DNS, false);
        addSignature("TLS", R"(^\x16\x03)", DPIResult::TLS, false);
        addSignature("SSH", R"(^SSH-)", DPIResult::SSH, false);
        addSignature("FTP", R"((?:^|\r\n)(USER |PASS |220 ))", DPIResult::FTP, true);
        addSignature("SMTP", R"((?:^|\r\n)(EHLO |HELO |MAIL FROM:|RCPT TO:))", DPIResult::SMTP, true);
        addSignature("QUIC", R"(^\xC3\xFF\x00)", DPIResult::QUIC, false);
    } catch (const std::exception& e) {
        std::cerr << "[DPIEngine] Error initializing signatures: " << e.what() << std::endl;
    }
}

void DPIEngine::addSignature(const std::string& name, const std::string& regex_str, DPIResult result, bool case_insensitive) {
    std::lock_guard<std::mutex> lock(mutex_);
    try {
        signatures.push_back({name, make_regex(regex_str, case_insensitive), result});
    } catch (const std::exception& e) {
        std::cerr << "[DPIEngine] Failed to add signature '" << name << "': " << e.what() << std::endl;
    }
}

void DPIEngine::removeSignature(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    signatures.erase(
        std::remove_if(signatures.begin(), signatures.end(),
            [&](const Signature& sig) { return sig.name == name; }),
        signatures.end()
    );
}

std::vector<std::string> DPIEngine::listSignatures() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> names;
    for (const auto& sig : signatures)
        names.push_back(sig.name);
    return names;
}

DPIResult DPIEngine::inspect(const uint8_t* payload, size_t len, std::string& matched_info) {
    if (!payload || len == 0) {
        matched_info = "Empty payload";
        return DPIResult::NONE;
    }

    std::string data(reinterpret_cast<const char*>(payload), len);

    std::lock_guard<std::mutex> lock(mutex_);
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