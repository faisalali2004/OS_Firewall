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

Rule::Rule()
    : srcPort(0), dstPort(0), enabled(true) {}

Rule::Rule(const std::string& id, const std::string& desc, const std::string& action,
           const std::string& protocol, const std::string& srcIP, const std::string& dstIP,
           uint16_t srcPort, uint16_t dstPort, bool enabled)
    : id(id), description(desc), action(action), protocol(protocol), srcIP(srcIP), dstIP(dstIP),
      srcPort(srcPort), dstPort(dstPort), enabled(enabled) {}

std::string Rule::getId() const { return id; }
std::string Rule::getDescription() const { return description; }
std::string Rule::getAction() const { return action; }
std::string Rule::getProtocol() const { return protocol; }
std::string Rule::getSrcIP() const { return srcIP; }
std::string Rule::getDstIP() const { return dstIP; }
uint16_t Rule::getSrcPort() const { return srcPort; }
uint16_t Rule::getDstPort() const { return dstPort; }
bool Rule::isEnabled() const { return enabled; }
void Rule::setEnabled(bool en) { enabled = en; }

std::string Rule::serialize() const {
    std::ostringstream oss;
    oss << id << "," << description << "," << action << "," << protocol << "," << srcIP << "," << dstIP << ","
        << srcPort << "," << dstPort << "," << (enabled ? "1" : "0");
    return oss.str();
}

bool Rule::deserialize(const std::string& str) {
    std::istringstream iss(str);
    std::string srcPortStr, dstPortStr, enabledStr;
    std::getline(iss, id, ',');
    std::getline(iss, description, ',');
    std::getline(iss, action, ',');
    std::getline(iss, protocol, ',');
    std::getline(iss, srcIP, ',');
    std::getline(iss, dstIP, ',');
    std::getline(iss, srcPortStr, ',');
    std::getline(iss, dstPortStr, ',');
    std::getline(iss, enabledStr, ',');
    srcPort = static_cast<uint16_t>(srcPortStr.empty() ? 0 : std::stoi(srcPortStr));
    dstPort = static_cast<uint16_t>(dstPortStr.empty() ? 0 : std::stoi(dstPortStr));
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
    }
}

// --- LOG PACKETS TO FILE ---
void RuleManager::printPacketInfo(const Packet& pkt) const {
    std::ofstream log("packets.log", std::ios::app);
    if (!log.is_open()) {
        std::cerr << "[!] Could not open packets.log for writing!" << std::endl;
        return;
    }
    log << (pkt.direction == Direction::IN ? "[IN]  " : "[OUT] ")
        << pkt.srcIP << ":" << pkt.srcPort << " -> "
        << pkt.dstIP << ":" << pkt.dstPort
        << " | Proto: " << pkt.protocol
        << " | Size: " << pkt.size << " bytes"
        << std::endl;
    // Optionally, also print to console if you want:
    // std::cout << (pkt.direction == Direction::IN ? "[IN]  " : "[OUT] ")
    //           << pkt.srcIP << ":" << pkt.srcPort << " -> "
    //           << pkt.dstIP << ":" << pkt.dstPort
    //           << " | Proto: " << pkt.protocol
    //           << " | Size: " << pkt.size << " bytes"
    //           << std::endl;
}

void RuleManager::startPacketCapture(const std::string& iface) {
    if (!packetCapture) packetCapture = new PacketCapture();
    packetCapture->startCapture(iface, [this](const Packet& pkt) {
        this->printPacketInfo(pkt);
        this->inspectPacket(pkt);
        if (!this->ruleEngine->applyRules(pkt, rules)) {
            logger->log("[RuleEngine] Packet blocked by rule.");
            return;
        }
        this->trafficShaper->shape(pkt);
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