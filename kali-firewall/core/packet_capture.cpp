#include "packet_capture.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <atomic>
#include <csignal>

namespace {
constexpr size_t DEFAULT_BUF_SIZE = 0x10000; // 64KB
}

PacketCapture::PacketCapture()
    : nfqHandle(nullptr), queueHandle(nullptr), fd(-1), running(false) {}

PacketCapture::~PacketCapture() {
    stop();
}

bool PacketCapture::init(uint16_t queue_num, PacketHandler handler, size_t buf_size) {
    std::lock_guard<std::mutex> lock(mtx);

    if (running) {
        std::cerr << "[PacketCapture] Already running, stop first.\n";
        return false;
    }

    nfqHandle = nfq_open();
    if (!nfqHandle) {
        std::cerr << "[PacketCapture] nfq_open() failed\n";
        return false;
    }

    if (nfq_unbind_pf(nfqHandle, AF_INET) < 0) {
        std::cerr << "[PacketCapture] nfq_unbind_pf() failed\n";
        nfq_close(nfqHandle);
        nfqHandle = nullptr;
        return false;
    }

    if (nfq_bind_pf(nfqHandle, AF_INET) < 0) {
        std::cerr << "[PacketCapture] nfq_bind_pf() failed\n";
        nfq_close(nfqHandle);
        nfqHandle = nullptr;
        return false;
    }

    userHandler = handler;
    queueHandle = nfq_create_queue(nfqHandle, queue_num, &PacketCapture::internalCallback, this);
    if (!queueHandle) {
        std::cerr << "[PacketCapture] nfq_create_queue() failed\n";
        nfq_close(nfqHandle);
        nfqHandle = nullptr;
        return false;
    }

    if (nfq_set_mode(queueHandle, NFQNL_COPY_PACKET, 0xffff) < 0) {
        std::cerr << "[PacketCapture] nfq_set_mode() failed\n";
        nfq_destroy_queue(queueHandle);
        queueHandle = nullptr;
        nfq_close(nfqHandle);
        nfqHandle = nullptr;
        return false;
    }

    fd = nfq_fd(nfqHandle);
    bufferSize = buf_size ? buf_size : DEFAULT_BUF_SIZE;
    return true;
}

void PacketCapture::start() {
    std::lock_guard<std::mutex> lock(mtx);
    if (running || fd < 0) return;
    running = true;
    captureThread = std::thread([this]() {
        std::vector<char> buf(bufferSize, 0);
        while (running) {
            int rv = recv(fd, buf.data(), buf.size(), 0);
            if (rv >= 0) {
                int ret = nfq_handle_packet(nfqHandle, buf.data(), rv);
                if (ret < 0) {
                    std::cerr << "[PacketCapture] nfq_handle_packet() error: " << ret << std::endl;
                }
            } else if (running) {
                if (errno == EINTR) continue;
                std::cerr << "[PacketCapture] recv() failed: " << strerror(errno) << std::endl;
                break;
            }
        }
    });
}

void PacketCapture::stop() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (!running) return;
        running = false;
    }
    if (captureThread.joinable())
        captureThread.join();

    if (queueHandle) {
        nfq_destroy_queue(queueHandle);
        queueHandle = nullptr;
    }
    if (nfqHandle) {
        nfq_close(nfqHandle);
        nfqHandle = nullptr;
    }
    fd = -1;
}

int PacketCapture::internalCallback(struct nfq_q_handle* qh, struct nfgenmsg* nfmsg, struct nfq_data* nfa, void* data) {
    PacketCapture* self = static_cast<PacketCapture*>(data);
    if (self && self->userHandler)
        return self->userHandler(qh, nfmsg, nfa);
    // Default: accept if no handler
    uint32_t id = 0;
    struct nfqnl_msg_packet_hdr* ph = nfq_get_msg_packet_hdr(nfa);
    if (ph) id = ntohl(ph->packet_id);
    return nfq_set_verdict(qh, id, NF_ACCEPT, 0, nullptr);
}