#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include "rule_engine.h"
#include "dpi_engine.h"
#include <QObject>

class PacketCapture : public QObject {
    Q_OBJECT
public:
    PacketCapture();
    ~PacketCapture();

    bool init(uint16_t queue_num = 0, size_t buf_size = 4096);
    void start();
    void stop();

    void setRuleEngine(RuleEngine* re) { ruleEngine = re; }
    void setDPIEngine(DPIEngine* de) { dpiEngine = de; }

signals:
    void statsUpdated(int totalPackets, int blockedPackets, int memoryUsageKB);

private:
    static int internalCallback(struct nfq_q_handle* qh, struct nfgenmsg*, struct nfq_data* nfa, void* data);

    struct nfq_handle* nfqHandle;
    struct nfq_q_handle* queueHandle;
    int fd;
    size_t bufferSize;
    std::thread captureThread;
    std::mutex mtx;
    std::atomic<bool> running{false};

    RuleEngine* ruleEngine = nullptr;
    DPIEngine* dpiEngine = nullptr;

    std::atomic<int> totalPackets{0};
    std::atomic<int> blockedPackets{0};

    void captureLoop();
    int getCurrentMemoryUsageKB();
};