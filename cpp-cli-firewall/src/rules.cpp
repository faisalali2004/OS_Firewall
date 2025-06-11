#ifndef RULES_H
#define RULES_H

#include "rules.h"
#include "dpi_engine.h"
#include "logger.h"
#include "packet_capture.h"
#include "rule_engine.h"
#include "traffic_shaper.h"
#include <iostream>
#include <sstream>
#include <fstream>
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

// --- RuleManager Implementation ---

RuleManager::RuleManager()
    : dpiEngine(new DPIEngine()),
      logger(new Logger()),
      packetCapture(new PacketCapture()),
      ruleEngine(new RuleEngine()),
      trafficShaper(new TrafficShaper())
{}

RuleManager::~RuleManager() {
    stopPacketCapture();
    delete dpiEngine;
    delete logger;
    delete packetCapture;
    delete ruleEngine;
    delete trafficShaper;
}

void RuleManager::addRule(const Rule& rule) {
    rules.push_back(rule);
    logger->log("[+] Rule added: " + rule.getDescription());
}

bool RuleManager::removeRuleById(const std::string& ruleId) {
    auto it = std::remove_if(rules.begin(), rules.end(),
        [&ruleId](const Rule& rule) { return rule.getId() == ruleId; });
    if (it != rules.end()) {
        logger->log("[-] Rule removed: " + ruleId);
        rules.erase(it, rules.end());
        return true;
    }
    logger->log("[!] Rule not found: " + ruleId);
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

void RuleManager::inspectPacket(const Packet& packet) {
    if (dpiEngine->inspect(packet)) {
        logger->log("[DPI] Suspicious packet detected.");
        // Optionally take action, e.g., drop or log
    }
}

void RuleManager::printPacketInfo(const Packet& pkt) const {
    // Print to console
    std::cout << (pkt.direction == Direction::IN ? "[IN]  " : "[OUT] ")
              << pkt.srcIP << ":" << pkt.srcPort << " -> "
              << pkt.dstIP << ":" << pkt.dstPort
              << " | Proto: " << pkt.protocol << " | Size: " << pkt.size << " bytes"
              << std::endl;

    // Save to JSON file (append mode)
    std::ofstream jsonFile("packets.json", std::ios::app);
    if (jsonFile.is_open()) {
        jsonFile << "{"
                 << "\"direction\":\"" << (pkt.direction == Direction::IN ? "IN" : "OUT") << "\","
                 << "\"srcIP\":\"" << pkt.srcIP << "\","
                 << "\"srcPort\":" << pkt.srcPort << ","
                 << "\"dstIP\":\"" << pkt.dstIP << "\","
                 << "\"dstPort\":" << pkt.dstPort << ","
                 << "\"protocol\":\"" << pkt.protocol << "\","
                 << "\"size\":" << pkt.size
                 << "}," << std::endl;
        jsonFile.close();
    }
}

void RuleManager::startPacketCapture(const std::string& iface) {
    if (!packetCapture) packetCapture = new PacketCapture();
    packetCapture->startCapture(iface, [this](const Packet& pkt) {
        this->printPacketInfo(pkt); // Print and save incoming/outgoing packet info
        this->inspectPacket(pkt);   // DPI
        if (!this->ruleEngine->applyRules(pkt, rules)) {
            logger->log("[RuleEngine] Packet blocked by rule.");
            return;
        }
        this->trafficShaper->shape(pkt); // Traffic shaping
        // Forward/process packet as needed
    });
}

void RuleManager::stopPacketCapture() {
    if (packetCapture) {
        packetCapture->stopCapture();
    }
}

void RuleManager::clearRules() {
    rules.clear();
    logger->log("All rules cleared.");
}

void RuleManager::loadRules(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return;
    rules.clear();
    std::string line;
    while (std::getline(file, line)) {
        Rule rule;
        if (rule.deserialize(line)) {
            rules.push_back(rule);
        }
    }
    logger->log("Rules loaded from disk.");
}

void RuleManager::saveRules(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) return;
    for (const auto& rule : rules) {
        file << rule.serialize() << std::endl;
    }
    logger->log("Rules saved to disk.");
}
#endif 