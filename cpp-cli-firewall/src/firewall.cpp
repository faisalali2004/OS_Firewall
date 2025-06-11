#include "firewall.h"
#include "rules.h"
#include "utils.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <ctime>

Firewall::Firewall() : isRunning(false) {
    loadRules();
}

Firewall::~Firewall() {
    saveRules();
}

void Firewall::start() {
    if (isRunning) {
        std::cerr << "[!] Firewall is already running." << std::endl;
        return;
    }
    isRunning = true;
    logEvent("Firewall started.");
    std::cout << "[+] Firewall started." << std::endl;
}

void Firewall::stop() {
    if (!isRunning) {
        std::cerr << "[!] Firewall is not running." << std::endl;
        return;
    }
    isRunning = false;
    logEvent("Firewall stopped.");
    std::cout << "[-] Firewall stopped." << std::endl;
}

bool Firewall::isRunningStatus() const {
    return isRunning;
}

bool Firewall::addRule(const Rule& rule) {
    rules.push_back(rule);
    logEvent("Rule added: " + rule.getDescription());
    std::cout << "[+] Rule added: " << rule.getDescription() << std::endl;
    return saveRules();
}

bool Firewall::removeRuleById(const std::string& ruleId) {
    int idx = findRuleIndexById(ruleId);
    if (idx == -1) {
        std::cerr << "[!] Rule not found: " << ruleId << std::endl;
        return false;
    }
    logEvent("Rule removed: " + rules[idx].getDescription());
    rules.erase(rules.begin() + idx);
    std::cout << "[-] Rule removed: " << ruleId << std::endl;
    return saveRules();
}

bool Firewall::removeRuleByIndex(size_t index) {
    if (index >= rules.size()) {
        std::cerr << "[!] Invalid rule index." << std::endl;
        return false;
    }
    logEvent("Rule removed: " + rules[index].getDescription());
    rules.erase(rules.begin() + index);
    std::cout << "[-] Rule removed at index " << index << std::endl;
    return saveRules();
}

bool Firewall::enableRuleById(const std::string& ruleId) {
    int idx = findRuleIndexById(ruleId);
    if (idx == -1) {
        std::cerr << "[!] Rule not found: " << ruleId << std::endl;
        return false;
    }
    rules[idx].setEnabled(true);
    logEvent("Rule enabled: " + rules[idx].getDescription());
    std::cout << "[+] Rule enabled: " << ruleId << std::endl;
    return saveRules();
}

bool Firewall::disableRuleById(const std::string& ruleId) {
    int idx = findRuleIndexById(ruleId);
    if (idx == -1) {
        std::cerr << "[!] Rule not found: " << ruleId << std::endl;
        return false;
    }
    rules[idx].setEnabled(false);
    logEvent("Rule disabled: " + rules[idx].getDescription());
    std::cout << "[-] Rule disabled: " << ruleId << std::endl;
    return saveRules();
}

bool Firewall::enableRuleByIndex(size_t index) {
    if (index >= rules.size()) {
        std::cerr << "[!] Invalid rule index." << std::endl;
        return false;
    }
    rules[index].setEnabled(true);
    logEvent("Rule enabled: " + rules[index].getDescription());
    std::cout << "[+] Rule enabled at index " << index << std::endl;
    return saveRules();
}

bool Firewall::disableRuleByIndex(size_t index) {
    if (index >= rules.size()) {
        std::cerr << "[!] Invalid rule index." << std::endl;
        return false;
    }
    rules[index].setEnabled(false);
    logEvent("Rule disabled: " + rules[index].getDescription());
    std::cout << "[-] Rule disabled at index " << index << std::endl;
    return saveRules();
}

bool Firewall::editRule(const std::string& ruleId, const Rule& newRule) {
    int idx = findRuleIndexById(ruleId);
    if (idx == -1) {
        std::cerr << "[!] Rule not found: " << ruleId << std::endl;
        return false;
    }
    rules[idx] = newRule;
    logEvent("Rule edited: " + ruleId);
    std::cout << "[*] Rule edited: " << ruleId << std::endl;
    return saveRules();
}

void Firewall::listRules() const {
    if (rules.empty()) {
        std::cout << "No rules defined." << std::endl;
        return;
    }
    std::cout << "Current firewall rules:" << std::endl;
    for (size_t i = 0; i < rules.size(); ++i) {
        std::cout << i << ": " << rules[i].getId() << " | " << rules[i].getDescription()
                  << " | " << rules[i].getAction()
                  << " [" << (rules[i].isEnabled() ? "ENABLED" : "DISABLED") << "]" << std::endl;
    }
}

const Rule* Firewall::getRuleById(const std::string& ruleId) const {
    for (const auto& rule : rules) {
        if (rule.getId() == ruleId) return &rule;
    }
    return nullptr;
}

void Firewall::status() const {
    std::cout << "Firewall status: " << (isRunning ? "RUNNING" : "STOPPED") << std::endl;
    std::cout << "Number of rules: " << rules.size() << std::endl;
}

bool Firewall::saveRules() const {
    std::ofstream outFile("rules.dat");
    if (!outFile.is_open()) {
        std::cerr << "[!] Failed to save rules." << std::endl;
        return false;
    }
    for (const auto& rule : rules) {
        outFile << rule.serialize() << std::endl;
    }
    outFile.close();
    return true;
}

bool Firewall::loadRules() {
    std::ifstream inFile("rules.dat");
    if (!inFile.is_open()) {
        std::cerr << "[!] No rules file found. Starting with empty rule set." << std::endl;
        return false;
    }
    std::string line;
    rules.clear();
    while (std::getline(inFile, line)) {
        Rule rule;
        if (rule.deserialize(line)) {
            rules.push_back(rule);
        }
    }
    inFile.close();
    return true;
}

void Firewall::logEvent(const std::string& event) const {
    logMessage(event, LogLevel::INFO);
}

int Firewall::findRuleIndexById(const std::string& ruleId) const {
    for (size_t i = 0; i < rules.size(); ++i) {
        if (rules[i].getId() == ruleId) return static_cast<int>(i);
    }
    return -1;
}