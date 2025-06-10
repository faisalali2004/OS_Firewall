#include "dpi_engine.h"
#include <cstring>

DPIEngine::DPIEngine() {
    // Add basic protocol signatures
    addSignature("HTTP", R"(^(GET|POST|HEAD|PUT|DELETE|OPTIONS|CONNECT|TRACE|PATCH) )");
    addSignature("DNS",  R"(^.{2}\x01\x00)"); // Simple DNS query (not robust)
    addSignature("TLS",  R"(^\x16\x03)");
    addSignature("SSH",  R"(^SSH-)");
}

void DPIEngine::addSignature(const std::string& name, const std::string& pattern) {
    signatures.push_back({name, std::regex(pattern, std::regex::ECMAScript)});
}

DPIResult DPIEngine::inspect(const uint8_t* payload, size_t len, std::string& matched_info) {
    std::string data(reinterpret_cast<const char*>(payload), len > 32 ? 32 : len); // Only inspect first 32 bytes
    for (const auto& sig : signatures) {
        if (std::regex_search(data, sig.pattern)) {
            matched_info = sig.name;
            if (sig.name == "HTTP") return DPIResult::HTTP;
            if (sig.name == "DNS")  return DPIResult::DNS;
            if (sig.name == "TLS")  return DPIResult::TLS;
            if (sig.name == "SSH")  return DPIResult::SSH;
        }
    }
    matched_info.clear();
    return DPIResult::NONE;
}