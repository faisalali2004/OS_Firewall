#ifndef RULES_H
#define RULES_H

#include <string>
#include <vector>

class Rule {
public:
    Rule();
    Rule(const std::string& id, const std::string& desc, const std::string& action, bool enabled = true);

    std::string getId() const;
    std::string getDescription() const;
    std::string getAction() const;
    bool isEnabled() const;

    void setEnabled(bool en);

    // Serialization for persistence
    std::string serialize() const;
    bool deserialize(const std::string& str);

private:
    std::string id;
    std::string description;
    std::string action;
    bool enabled;
};

// Optional: RuleManager for managing a collection of rules
class RuleManager {
public:
    RuleManager();

    void addRule(const Rule& rule);
    bool removeRuleById(const std::string& ruleId);
    void listRules() const;

    // You can add more advanced management functions as needed
private:
    std::vector<Rule> rules;
};

#endif // RULES_H