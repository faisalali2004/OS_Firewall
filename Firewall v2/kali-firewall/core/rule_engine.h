#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include "packet_info.h"

// Structure for a firewall rule
struct Rule {
    QString srcIp;
    QString dstIp;
    QString srcPort;
    QString dstPort;
    QString action; // "allow" or "block"
};

class RuleEngine : public QObject {
    Q_OBJECT
public:
    explicit RuleEngine(QObject* parent = nullptr, const QString& rulesPath = QString());
    ~RuleEngine();

    void setInteractiveMode(bool enabled);
    QString decide(const PacketInfo& pkt);
    void addRule(const Rule& rule);
    void removeRule(int index);
    void clearRules();
    QList<Rule> getRules() const;
    bool loadRules(const QString& path);
    bool saveRules(const QString& path) const;

    // For packet_capture.cpp integration
    bool shouldBlock(const std::string& src_ip,
                     const std::string& dst_ip,
                     int src_port,
                     int dst_port,
                     const std::string& protocol,
                     const unsigned char* payload,
                     int payload_len);

signals:
    void userDecisionNeeded(const PacketInfo& pkt);
    void userDecisionReceived(const QString& action);

private:
    bool matchRule(const Rule& rule, const PacketInfo& pkt) const;
    QString askUserForDecision(const PacketInfo& pkt);

    QList<Rule> rules;
    QString rulesPath;
    bool interactiveMode;
    mutable QMutex mutex;
};