#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QMutex>

// Structure representing a firewall rule
struct Rule {
    QString srcIp;
    QString dstIp;
    QString srcPort;
    QString dstPort;
    QString action; // "allow" or "block"
};

// Structure representing a packet's relevant info
struct PacketInfo {
    QString srcIp;
    QString dstIp;
    QString srcPort;
    QString dstPort;
};

class RuleEngine : public QObject {
    Q_OBJECT

public:
    explicit RuleEngine(QObject* parent = nullptr, const QString& rulesPath = "../config/default_rules.json");
    ~RuleEngine();

    // Main decision function: returns "allow" or "block"
    QString decide(const PacketInfo& pkt);

    // Rule management
    void addRule(const Rule& rule);
    void removeRule(int index);
    void clearRules();
    QList<Rule> getRules() const;

    // Persistence
    bool loadRules(const QString& path);
    bool saveRules(const QString& path) const;

    // Interactive mode
    void setInteractiveMode(bool enabled);

signals:
    // Emitted when a new/unknown packet needs a user decision
    void userDecisionNeeded(const PacketInfo& pkt);

public slots:
    // Call this from GUI when user makes a decision ("allow" or "block")
    void userDecisionReceived(const QString& action);

private:
    // Helper for matching rules
    bool matchRule(const Rule& rule, const PacketInfo& pkt) const;

    // Interactive mode handler (blocks until user responds or timeout)
    QString askUserForDecision(const PacketInfo& pkt);

    QList<Rule> rules;
    QString rulesPath;
    bool interactiveMode;
    mutable QMutex mutex;
};