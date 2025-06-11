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

void Firewall::addRule(const Rule& rule) {
    rules.push_back(rule);
    logEvent("Rule added: " + rule.getDescription());
    std::cout << "[+] Rule added: " << rule.getDescription() << std::endl;
    saveRules();
}

void Firewall::removeRule(size_t index) {
    if (index >= rules.size()) {
        std::cerr << "[!] Invalid rule index." << std::endl;
        return;
    }
    logEvent("Rule removed: " + rules[index].getDescription());
    rules.erase(rules.begin() + index);
    std::cout << "[-] Rule removed at index " << index << std::endl;
    saveRules();
}

void Firewall::enableRule(size_t index) {
    if (index >= rules.size()) {
        std::cerr << "[!] Invalid rule index." << std::endl;
        return;
    }
    rules[index].setEnabled(true);
    logEvent("Rule enabled: " + rules[index].getDescription());
    std::cout << "[+] Rule enabled at index " << index << std::endl;
    saveRules();
}

void Firewall::disableRule(size_t index) {
    if (index >= rules.size()) {
        std::cerr << "[!] Invalid rule index." << std::endl;
        return;
    }
    rules[index].setEnabled(false);
    logEvent("Rule disabled: " + rules[index].getDescription());
    std::cout << "[-] Rule disabled at index " << index << std::endl;
    saveRules();
}

void Firewall::listRules() const {
    if (rules.empty()) {
        std::cout << "No rules defined." << std::endl;
        return;
    }
    std::cout << "Current firewall rules:" << std::endl;
    for (size_t i = 0; i < rules.size(); ++i) {
        std::cout << i << ": " << rules[i].getDescription()
                  << " [" << (rules[i].isEnabled() ? "ENABLED" : "DISABLED") << "]" << std::endl;
    }
}

void Firewall::status() const {
    std::cout << "Firewall status: " << (isRunning ? "RUNNING" : "STOPPED") << std::endl;
    std::cout << "Number of rules: " << rules.size() << std::endl;
}

void Firewall::logEvent(const std::string& event) const {
    std::ofstream logFile("firewall.log", std::ios::app);
    if (logFile.is_open()) {
        std::time_t now = std::time(nullptr);
        logFile << std::ctime(&now) << ": " << event << std::endl;
        logFile.close();
    }
}

void Firewall::saveRules() const {
    std::ofstream outFile("rules.dat");
    for (const auto& rule : rules) {
        outFile << rule.serialize() << std::endl;
    }
}

void Firewall::loadRules() {
    std::ifstream inFile("rules.dat");
    std::string line;
    rules.clear();
    while (std::getline(inFile, line)) {
        Rule rule;
        if (rule.deserialize(line)) {
            rules.push_back(rule);
        }
    }
}