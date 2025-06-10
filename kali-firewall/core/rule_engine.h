#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

enum class RuleAction { ALLOW, DROP, LOG, SHAPE };

struct Rule {
    std::string src_ip, dst_ip;
    int src_port, dst_port;
    std::string protocol;
    RuleAction action;
};

class RuleEngine {
public:
    bool loadRules(const std::string& path);
    RuleAction evaluate(const std::string& src_ip, int src_port,
                        const std::string& dst_ip, int dst_port,
                        const std::string& protocol);
    std::vector<Rule> rules;
    static RuleAction actionFromString(const std::string& s);
};