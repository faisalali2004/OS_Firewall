#pragma once

#include <functional>
#include <thread>
#include <atomic>
#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>

class PacketCapture {
public:
    using PacketHandler = std::function<int(struct nfq_q_handle*, struct nfgenmsg*, struct nfq_data*)>;

    PacketCapture();
    ~PacketCapture();

    bool init(uint16_t queue_num, PacketHandler handler);
    void start();
    void stop();

private:
    static int internalCallback(struct nfq_q_handle*, struct nfgenmsg*, struct nfq_data*, void*);
    struct nfq_handle* nfqHandle;
    struct nfq_q_handle* queueHandle;
    int fd;
    std::thread captureThread;
    std::atomic<bool> running;
    PacketHandler userHandler;
};