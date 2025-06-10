#pragma once

#include <string>
#include <vector>
#include <regex>

enum class DPIResult { NONE, HTTP, DNS, TLS, SSH };

class DPIEngine {
public:
    DPIEngine();
    DPIResult inspect(const uint8_t* payload, size_t len, std::string& matched_info);
    void addSignature(const std::string& name, const std::string& pattern);

private:
    struct Signature {
        std::string name;
        std::regex pattern;
    };
    std::vector<Signature> signatures;
};