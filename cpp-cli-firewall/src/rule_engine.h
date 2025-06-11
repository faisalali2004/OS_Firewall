#ifndef RULE_ENGINE_H
#define RULE_ENGINE_H

#include "packet.h"
#include "rules.h"
#include <vector>

class RuleEngine {
public:
    RuleEngine() = default;
    ~RuleEngine() = default;
    void applyRules(const Packet& pkt, const std::vector<Rule>& rules) {
        // Apply rules to the packet
        // This is a stub for demonstration.
    }
};

#endif // RULE_ENGINE_H