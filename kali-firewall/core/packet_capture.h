#pragma once

#include <QObject>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <libnetfilter_queue/libnetfilter_queue.h>

class RuleEngine;
class DPIEngine;

class PacketCapture : public QObject {
    Q_OBJECT
public:
    PacketCapture();
    ~PacketCapture();

    bool init(uint16_t queue_num = 0, size_t buf_size = 0x10000);
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
    std::atomic<bool> running;

    RuleEngine* ruleEngine;
    DPIEngine* dpiEngine;

    std::atomic<int> totalPackets;
    std::atomic<int> blockedPackets;

    void captureLoop();
    int getCurrentMemoryUsageKB();
};