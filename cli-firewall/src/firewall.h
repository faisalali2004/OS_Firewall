#ifndef FIREWALL_H
#define FIREWALL_H

#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <memory>

// Forward declarations
class RuleManager;
class Rule;

class Firewall {
public:
    Firewall();
    ~Firewall();

    // Firewall control
    void start();
    void stop();
    bool isRunning() const;

    // Rule management
    void addRule(const Rule& rule);
    void removeRule(const std::string& ruleId);
    void listRules() const;
    void clearRules();

    // Logging
    void logEvent(const std::string& event) const;
    void showLogs() const;

    // Packet capture interface
    void setCaptureInterface(const std::string& iface);
    std::string getCaptureInterface() const;

    // Persistence
    void loadRules();
    void saveRules() const;

    // DPI and traffic shaping (stubs for extensibility)
    void enableDPI(bool enable);
    void enableTrafficShaping(bool enable);

private:
    std::atomic<bool> running_;
    std::string captureInterface_;
    std::unique_ptr<RuleManager> ruleManager_;
    mutable std::mutex ruleMutex_;
    bool dpiEnabled_;
    bool trafficShapingEnabled_;

    // Helper for internal state
    void initialize();
    void shutdown();
};

#endif // FIREWALL_H