#ifndef RULES_H
#define RULES_H

#include <string>
#include <vector>
#include "packet.h"
#include "dpi_engine.h"
#include "logger.h"
#include "packet_capture.h"

// Forward declarations
class RuleEngine;
class TrafficShaper;

class Rule {
public:
    Rule();
    Rule(const std::string& id, const std::string& desc, const std::string& action,
         const std::string& protocol = "", const std::string& srcIP = "", const std::string& dstIP = "",
         uint16_t srcPort = 0, uint16_t dstPort = 0, bool enabled = true);

    std::string getId() const;
    std::string getDescription() const;
    std::string getAction() const;
    std::string getProtocol() const;
    std::string getSrcIP() const;
    std::string getDstIP() const;
    uint16_t getSrcPort() const;
    uint16_t getDstPort() const;
    bool isEnabled() const;

    void setEnabled(bool en);

    std::string serialize() const;
    bool deserialize(const std::string& str);

private:
    std::string id;
    std::string description;
    std::string action;
    std::string protocol;
    std::string srcIP;
    std::string dstIP;
    uint16_t srcPort;
    uint16_t dstPort;
    bool enabled;
};

class RuleManager {
public:
    RuleManager();
    ~RuleManager();

    void addRule(const Rule& rule);
    bool removeRuleById(const std::string& ruleId);
    void listRules() const;

    void inspectPacket(const Packet& packet);
    void printPacketInfo(const Packet& pkt) const;
    void startPacketCapture(const std::string& iface);
    void stopPacketCapture();
    void clearRules();
    void loadRules(const std::string& filename);
    void saveRules(const std::string& filename) const;

    const std::vector<Rule>& getRules() const { return rules; }

private:
    std::vector<Rule> rules;
    DPIEngine* dpiEngine;
    Logger* logger;
    PacketCapture* packetCapture;
    RuleEngine* ruleEngine;
    TrafficShaper* trafficShaper;
};

#endif // RULES_H