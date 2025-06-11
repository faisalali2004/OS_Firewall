#include "rules.h"
#include <iostream>
#include <sstream>
#include <algorithm>

// --- Rule Implementation ---

Rule::Rule() : enabled(true) {}

Rule::Rule(const std::string& id, const std::string& desc, const std::string& action, bool enabled)
    : id(id), description(desc), action(action), enabled(enabled) {}

std::string Rule::getId() const { return id; }
std::string Rule::getDescription() const { return description; }
std::string Rule::getAction() const { return action; }
bool Rule::isEnabled() const { return enabled; }

void Rule::setEnabled(bool en) { enabled = en; }

std::string Rule::serialize() const {
    std::ostringstream oss;
    oss << id << "," << description << "," << action << "," << (enabled ? "1" : "0");
    return oss.str();
}

bool Rule::deserialize(const std::string& str) {
    std::istringstream iss(str);
    std::getline(iss, id, ',');
    std::getline(iss, description, ',');
    std::getline(iss, action, ',');
    std::string enabledStr;
    std::getline(iss, enabledStr, ',');
    enabled = (enabledStr == "1" || enabledStr == "true");
    return !id.empty() && !action.empty();
}

// --- RuleManager Implementation (optional, if you want to use it) ---

RuleManager::RuleManager() {}

void RuleManager::addRule(const Rule& rule) {
    rules.push_back(rule);
    std::cout << "[+] Rule added: " << rule.getDescription() << std::endl;
}

bool RuleManager::removeRuleById(const std::string& ruleId) {
    auto it = std::remove_if(rules.begin(), rules.end(),
        [&ruleId](const Rule& rule) { return rule.getId() == ruleId; });
    if (it != rules.end()) {
        rules.erase(it, rules.end());
        std::cout << "[-] Rule removed: " << ruleId << std::endl;
        return true;
    }
    std::cout << "[!] Rule not found: " << ruleId << std::endl;
    return false;
}

void RuleManager::listRules() const {
    if (rules.empty()) {
        std::cout << "No rules defined." << std::endl;
        return;
    }
    std::cout << "Current firewall rules:" << std::endl;
    for (const auto& rule : rules) {
        std::cout << " - " << rule.getId() << ": " << rule.getDescription()
                  << " [" << rule.getAction() << "] "
                  << (rule.isEnabled() ? "[ENABLED]" : "[DISABLED]") << std::endl;
    }
}