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
    : dpiEngine(new DPIEngine()), logger(new Logger()), packetCapture(new PacketCapture()),
      ruleEngine(new RuleEngine()), trafficShaper(new TrafficShaper())
{}

RuleManager::~RuleManager() {
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

// --- DPI Engine Integration Example ---
void RuleManager::inspectPacket(const Packet& packet) {
    if (dpiEngine->inspect(packet)) {
        logger->log("[DPI] Suspicious packet detected.");
        // Take action, e.g., drop or log
    }
}

// --- Print Packet Info (Incoming/Outgoing) and Save to JSON ---
void RuleManager::printPacketInfo(const Packet& pkt) const {
    // Print to console
    std::cout << (pkt.direction == Packet::Direction::IN ? "[IN]  " : "[OUT] ")
              << pkt.srcIP << ":" << pkt.srcPort << " -> "
              << pkt.dstIP << ":" << pkt.dstPort
              << " | Proto: " << pkt.protocol << " | Size: " << pkt.size << " bytes"
              << std::endl;

    // Save to JSON file (append mode)
    std::ofstream jsonFile("packets.json", std::ios::app);
    if (jsonFile.is_open()) {
        jsonFile << "{"
                 << "\"direction\":\"" << (pkt.direction == Packet::Direction::IN ? "IN" : "OUT") << "\","
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

// --- Packet Capture Integration Example ---
void RuleManager::startPacketCapture(const std::string& iface) {
    packetCapture->startCapture(iface, [this](const Packet& pkt) {
        this->printPacketInfo(pkt); // Print and save incoming/outgoing packet info
        this->inspectPacket(pkt);   // DPI
        this->ruleEngine->applyRules(pkt, rules); // Apply rules
        this->trafficShaper->shape(pkt); // Traffic shaping
    });
}