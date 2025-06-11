#pragma once
#include "packet.h"
#include "rules.h"
#include <vector>
#include <iostream>

class RuleEngine {
public:
    // Returns true if packet is allowed, false if blocked
    bool applyRules(const Packet& pkt, const std::vector<Rule>& rules) {
        for (const auto& rule : rules) {
            if (!rule.isEnabled()) continue;
            // Example: match by protocol, src/dst IP, port
            if ((!rule.getProtocol().empty() && rule.getProtocol() != pkt.protocol) ||
                (!rule.getSrcIP().empty() && rule.getSrcIP() != pkt.srcIP) ||
                (!rule.getDstIP().empty() && rule.getDstIP() != pkt.dstIP) ||
                (rule.getSrcPort() != 0 && rule.getSrcPort() != pkt.srcPort) ||
                (rule.getDstPort() != 0 && rule.getDstPort() != pkt.dstPort)) {
                continue; // No match, check next rule
            }
            // Action: block or allow
            if (rule.getAction() == "block") {
                std::cout << "[RuleEngine] Blocked packet: " << pkt.summary() << " by rule: " << rule.getDescription() << std::endl;
                return false;
            } else if (rule.getAction() == "allow") {
                std::cout << "[RuleEngine] Allowed packet: " << pkt.summary() << " by rule: " << rule.getDescription() << std::endl;
                return true;
            }
        }
        // Default: allow if no rule matches
        return true;
    }
};