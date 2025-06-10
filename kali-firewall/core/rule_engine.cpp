#include "rule_engine.h
#include <fstream>
#include <algorithm>

using json = nlohmann::json;

RuleAction RuleEngine::actionFromString(const std::string& s) {
    if (s == "ALLOW") return RuleAction::ALLOW;
    if (s == "DROP") return RuleAction::DROP;
    if (s == "LOG") return RuleAction::LOG;
    if (s == "SHAPE") return RuleAction::SHAPE;
    return RuleAction::DROP;
}

bool RuleEngine::loadRules(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    json j; f >> j;
    rules.clear();
    for (const auto& item : j) {
        Rule r;
        r.src_ip = item.value("src_ip", "0.0.0.0/0");
        r.dst_ip = item.value("dst_ip", "0.0.0.0/0");
        r.src_port = item.value("src_port", 0);
        r.dst_port = item.value("dst_port", 0);
        r.protocol = item.value("protocol", "ANY");
        r.action = actionFromString(item.value("action", "DROP"));
        rules.push_back(r);
    }
    return true;
}

RuleAction RuleEngine::evaluate(const std::string& src_ip, int src_port,
                                const std::string& dst_ip, int dst_port,
                                const std::string& protocol) {
    for (const auto& rule : rules) {
        bool match = true;
        if (rule.src_ip != "0.0.0.0/0" && rule.src_ip != src_ip) match = false;
        if (rule.dst_ip != "0.0.0.0/0" && rule.dst_ip != dst_ip) match = false;
        if (rule.src_port != 0 && rule.src_port != src_port) match = false;
        if (rule.dst_port != 0 && rule.dst_port != dst_port) match = false;
        if (rule.protocol != "ANY" && rule.protocol != protocol) match = false;
        if (match) return rule.action;
    }
    return RuleAction::ALLOW; // Default allow
}