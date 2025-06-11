#ifndef FIREWALL_H
#define FIREWALL_H

#include <string>
#include <vector>
#include "rules.h"

class Firewall {
public:
    Firewall();
    ~Firewall();

    void start();
    void stop();
    bool isRunningStatus() const;

    // Rule management
    bool addRule(const Rule& rule);
    bool removeRuleById(const std::string& ruleId);
    bool removeRuleByIndex(size_t index);
    bool enableRuleById(const std::string& ruleId);
    bool disableRuleById(const std::string& ruleId);
    bool enableRuleByIndex(size_t index);
    bool disableRuleByIndex(size_t index);
    bool editRule(const std::string& ruleId, const Rule& newRule);

    // Query
    void listRules() const;
    const Rule* getRuleById(const std::string& ruleId) const;
    void status() const;

    // Persistence
    bool saveRules() const;
    bool loadRules();

    // Logging
    void logEvent(const std::string& event) const;

private:
    std::vector<Rule> rules;
    bool isRunning;

    int findRuleIndexById(const std::string& ruleId) const;
};

#endif // FIREWALL_H