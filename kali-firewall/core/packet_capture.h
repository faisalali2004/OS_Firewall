#pragma once

#include <thread>
#include <mutex>
#include <vector>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include "rule_engine.h"
#include "dpi_engine.h"

using PacketHandler = int(*)(struct nfq_q_handle*, struct nfgenmsg*, struct nfq_data*, void*);

class PacketCapture {
public:
    PacketCapture();
    ~PacketCapture();

    bool init(uint16_t queue_num = 0, PacketHandler handler = nullptr, size_t buf_size = 0);
    void start();
    void stop();

    void setRuleEngine(RuleEngine* re) { ruleEngine = re; }
    void setDPIEngine(DPIEngine* de) { dpiEngine = de; }

private:
    static int internalCallback(struct nfq_q_handle* qh, struct nfgenmsg*, struct nfq_data* nfa, void* data);

    struct nfq_handle* nfqHandle;
    struct nfq_q_handle* queueHandle;
    int fd;
    size_t bufferSize;
    std::thread captureThread;
    std::mutex mtx;
    bool running;

    PacketHandler userHandler;

    // Integration with RuleEngine and DPIEngine
    RuleEngine* ruleEngine;
    DPIEngine* dpiEngine;
};