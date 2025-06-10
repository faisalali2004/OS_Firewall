#pragma once
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>

using PacketHandler = int(*)(struct nfq_q_handle*, struct nfgenmsg*, struct nfq_data*);

class PacketCapture {
public:
    PacketCapture();
    ~PacketCapture();

    bool init(uint16_t queue_num, PacketHandler handler, size_t buf_size = 0);
    void start();
    void stop();

private:
    static int internalCallback(struct nfq_q_handle* qh, struct nfgenmsg* nfmsg, struct nfq_data* nfa, void* data);

    struct nfq_handle* nfqHandle;
    struct nfq_q_handle* queueHandle;
    int fd;
    std::thread captureThread;
    std::atomic<bool> running;
    PacketHandler userHandler;
    size_t bufferSize;
    std::mutex mtx;
};