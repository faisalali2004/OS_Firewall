#include "firewall.h"
#include "rules.h"
#include <fstream>
#include <iostream>
#include <ctime>
#include <iomanip>

Firewall::Firewall()
    : running_(false),
      captureInterface_("any"),
      ruleManager_(std::make_unique<RuleManager>()),
      dpiEnabled_(false),
      trafficShapingEnabled_(false) {
    initialize();
}

Firewall::~Firewall() {
    shutdown();
}

void Firewall::initialize() {
    loadRules();
    logEvent("Firewall initialized.");
}

void Firewall::shutdown() {
    stop();
    saveRules();
    logEvent("Firewall shutdown.");
}

void Firewall::start() {
    if (running_) {
        std::cerr << "[!] Firewall is already running." << std::endl;
        return;
    }
    running_ = true;
    logEvent("Firewall started.");
    std::cout << "[+] Firewall started on interface: " << captureInterface_ << std::endl;

    // Start packet capture and DPI if enabled
    if (ruleManager_) {
        ruleManager_->startPacketCapture(captureInterface_);
    }
    if (dpiEnabled_) {
        std::cout << "[*] DPI enabled." << std::endl;
        logEvent("DPI enabled.");
    }
    if (trafficShapingEnabled_) {
        std::cout << "[*] Traffic shaping enabled." << std::endl;
        logEvent("Traffic shaping enabled.");
    }
}

void Firewall::stop() {
    if (!running_) {
        std::cerr << "[!] Firewall is not running." << std::endl;
        return;
    }
    running_ = false;
    if (ruleManager_) {
        ruleManager_->stopPacketCapture();
    }
    logEvent("Firewall stopped.");
    std::cout << "[+] Firewall stopped." << std::endl;
}

bool Firewall::isRunning() const {
    return running_;
}

// Rule management
void Firewall::addRule(const Rule& rule) {
    std::lock_guard<std::mutex> lock(ruleMutex_);
    if (ruleManager_) {
        ruleManager_->addRule(rule);
        logEvent("Rule added: " + rule.getDescription());
        std::cout << "[+] Rule added: " << rule.getDescription() << std::endl;
    }
}

void Firewall::removeRule(const std::string& ruleId) {
    std::lock_guard<std::mutex> lock(ruleMutex_);
    if (ruleManager_) {
        ruleManager_->removeRuleById(ruleId);
        logEvent("Rule removed: ID " + ruleId);
        std::cout << "[-] Rule removed: ID " << ruleId << std::endl;
    }
}

void Firewall::listRules() const {
    std::lock_guard<std::mutex> lock(ruleMutex_);
    if (ruleManager_) {
        ruleManager_->listRules();
    }
}

void Firewall::clearRules() {
    std::lock_guard<std::mutex> lock(ruleMutex_);
    if (ruleManager_) {
        ruleManager_->clearRules();
        logEvent("All rules cleared.");
        std::cout << "[*] All rules cleared." << std::endl;
    }
}

// Logging
void Firewall::logEvent(const std::string& event) const {
    std::ofstream logFile("firewall.log", std::ios::app);
    if (logFile) {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        logFile << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] " << event << std::endl;
    }
}

void Firewall::showLogs() const {
    std::ifstream logFile("firewall.log");
    if (!logFile) {
        std::cout << "[!] No log file found." << std::endl;
        return;
    }
    std::string line;
    while (std::getline(logFile, line)) {
        std::cout << line << std::endl;
    }
}

// Packet capture interface
void Firewall::setCaptureInterface(const std::string& iface) {
    captureInterface_ = iface;
    logEvent("Capture interface set to: " + iface);
}

std::string Firewall::getCaptureInterface() const {
    return captureInterface_;
}

// Persistence
void Firewall::loadRules() {
    std::lock_guard<std::mutex> lock(ruleMutex_);
    if (ruleManager_) {
        ruleManager_->loadRules("rules.dat");
        logEvent("Rules loaded from disk.");
    }
}

void Firewall::saveRules() const {
    std::lock_guard<std::mutex> lock(ruleMutex_);
    if (ruleManager_) {
        ruleManager_->saveRules("rules.dat");
        logEvent("Rules saved to disk.");
    }
}

// DPI and traffic shaping (stubs)
void Firewall::enableDPI(bool enable) {
    dpiEnabled_ = enable;
    logEvent(std::string("DPI ") + (enable ? "enabled." : "disabled."));
}

void Firewall::enableTrafficShaping(bool enable) {
    trafficShapingEnabled_ = enable;
    logEvent(std::string("Traffic shaping ") + (enable ? "enabled." : "disabled."));
}