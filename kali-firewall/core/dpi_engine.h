#pragma once

#include <string>
#include <vector>
#include <regex>
#include <cstdint>

enum class DPIResult {
    NONE,
    HTTP,
    DNS,
    TLS,
    SSH,
    UNKNOWN
};

struct DPISignature {
    std::string name;
    std::regex pattern;
    DPIResult result;
};

class DPIEngine {
public:
    DPIEngine();
    DPIResult inspect(const uint8_t* payload, size_t len, std::string& matched_info);

    // For extensibility
    void addSignature(const std::string& name, const std::string& regex_str, DPIResult result);

private:
    std::vector<DPISignature> signatures;
};