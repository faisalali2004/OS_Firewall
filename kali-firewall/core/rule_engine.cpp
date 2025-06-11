#include "rule_engine.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QHostAddress>
#include <QMutexLocker>
#include <QDebug>
#include <QCoreApplication>
#include <QEventLoop>
#include <QTimer>

// --- CONSTRUCTOR / DESTRUCTOR ---
RuleEngine::RuleEngine(QObject* parent, const QString& rulesPath)
    : QObject(parent), rulesPath(rulesPath), interactiveMode(false)
{
    loadRules(rulesPath);
}

RuleEngine::~RuleEngine() {}

// --- INTERFACE ---

void RuleEngine::setInteractiveMode(bool enabled) {
    QMutexLocker locker(&mutex);
    interactiveMode = enabled;
}

bool RuleEngine::matchRule(const Rule& rule, const PacketInfo& pkt) const {
    auto matchField = [](const QString& ruleVal, const QString& pktVal) {
        return ruleVal.isEmpty() || ruleVal == "*" || ruleVal == pktVal;
    };
    return matchField(rule.srcIp, pkt.srcIp)
        && matchField(rule.dstIp, pkt.dstIp)
        && matchField(rule.srcPort, pkt.srcPort)
        && matchField(rule.dstPort, pkt.dstPort);
}

QString RuleEngine::decide(const PacketInfo& pkt) {
    QMutexLocker locker(&mutex);
    for (const Rule& rule : rules) {
        if (matchRule(rule, pkt)) {
            return rule.action.toLower();
        }
    }

    if (interactiveMode) {
        // Interactive mode: ask user via GUI and wait for response
        QString userDecision = askUserForDecision(pkt);
        if (!userDecision.isEmpty()) {
            // Add rule for future packets
            Rule newRule;
            newRule.srcIp = pkt.srcIp;
            newRule.dstIp = pkt.dstIp;
            newRule.srcPort = pkt.srcPort;
            newRule.dstPort = pkt.dstPort;
            newRule.action = userDecision;
            rules.push_front(newRule);
            saveRules(rulesPath); // Persist new rule
            return userDecision;
        }
    }
    return "block"; // Default action
}

void RuleEngine::addRule(const Rule& rule) {
    QMutexLocker locker(&mutex);
    rules.push_front(rule);
    saveRules(rulesPath);
}

void RuleEngine::removeRule(int index) {
    QMutexLocker locker(&mutex);
    if (index >= 0 && index < rules.size())
        rules.removeAt(index);
    saveRules(rulesPath);
}

void RuleEngine::clearRules() {
    QMutexLocker locker(&mutex);
    rules.clear();
    saveRules(rulesPath);
}

QList<Rule> RuleEngine::getRules() const {
    QMutexLocker locker(&mutex);
    return rules;
}

bool RuleEngine::loadRules(const QString& path) {
    QMutexLocker locker(&mutex);
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open rule file:" << path;
        return false;
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isArray()) {
        qWarning() << "Failed to parse rules JSON:" << err.errorString();
        return false;
    }

    rules.clear();
    QJsonArray arr = doc.array();
    for (const QJsonValue& val : arr) {
        if (!val.isObject()) continue;
        QJsonObject obj = val.toObject();
        Rule rule;
        rule.srcIp = obj.value("src_ip").toString();
        rule.dstIp = obj.value("dst_ip").toString();
        rule.srcPort = obj.value("src_port").toString();
        rule.dstPort = obj.value("dst_port").toString();
        rule.action = obj.value("action").toString().toLower();
        rules.append(rule);
    }
    return true;
}

bool RuleEngine::saveRules(const QString& path) const {
    QMutexLocker locker(&mutex);
    QJsonArray arr;
    for (const Rule& rule : rules) {
        QJsonObject obj;
        obj["src_ip"] = rule.srcIp;
        obj["dst_ip"] = rule.dstIp;
        obj["src_port"] = rule.srcPort;
        obj["dst_port"] = rule.dstPort;
        obj["action"] = rule.action;
        arr.append(obj);
    }
    QJsonDocument doc(arr);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "Failed to open rule file for writing:" << path;
        return false;
    }
    file.write(doc.toJson());
    file.close();
    return true;
}

// --- INTERACTIVE MODE IMPLEMENTATION ---

// This function emits a signal and waits for the GUI to respond.
// It uses an event loop to block until the user makes a decision or timeout occurs.
QString RuleEngine::askUserForDecision(const PacketInfo& pkt) {
    QString decision;
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    // Connect a lambda to receive the user's decision from the GUI
    connect(this, &RuleEngine::userDecisionReceived, &loop, [&](const QString& userAction) {
        decision = userAction.toLower();
        loop.quit();
    });

    // Timeout after 30 seconds (auto-block)
    connect(&timer, &QTimer::timeout, &loop, [&]() {
        decision = "block";
        loop.quit();
    });

    emit userDecisionNeeded(pkt); // Signal to GUI to prompt user
    timer.start(30000); // 30 seconds
    loop.exec();

    return decision;
}

// --- REQUIRED FOR PACKET CAPTURE INTEGRATION ---
bool RuleEngine::shouldBlock(const std::string& src_ip,
                             const std::string& dst_ip,
                             int src_port,
                             int dst_port,
                             const std::string& protocol,
                             const unsigned char* /*payload*/,
                             int /*payload_len*/)
{
    PacketInfo pkt;
    pkt.srcIp = QString::fromStdString(src_ip);
    pkt.dstIp = QString::fromStdString(dst_ip);
    pkt.srcPort = QString::number(src_port);
    pkt.dstPort = QString::number(dst_port);
    pkt.protocol = QString::fromStdString(protocol);

    QString action = decide(pkt);
    return action == "block";
}