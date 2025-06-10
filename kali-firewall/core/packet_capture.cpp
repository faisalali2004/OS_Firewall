#include "packet_capture.h"
#include <iostream>
#include <cstring>
#include <unistd.h>

PacketCapture::PacketCapture()
    : nfqHandle(nullptr), queueHandle(nullptr), fd(-1), running(false) {}

PacketCapture::~PacketCapture() {
    stop();
}

bool PacketCapture::init(uint16_t queue_num, PacketHandler handler) {
    nfqHandle = nfq_open();
    if (!nfqHandle) {
        std::cerr << "Error: nfq_open() failed\n";
        return false;
    }

    if (nfq_unbind_pf(nfqHandle, AF_INET) < 0) {
        std::cerr << "Error: nfq_unbind_pf() failed\n";
        nfq_close(nfqHandle);
        return false;
    }

    if (nfq_bind_pf(nfqHandle, AF_INET) < 0) {
        std::cerr << "Error: nfq_bind_pf() failed\n";
        nfq_close(nfqHandle);
        return false;
    }

    userHandler = handler;
    queueHandle = nfq_create_queue(nfqHandle, queue_num, &PacketCapture::internalCallback, this);
    if (!queueHandle) {
        std::cerr << "Error: nfq_create_queue() failed\n";
        nfq_close(nfqHandle);
        return false;
    }

    if (nfq_set_mode(queueHandle, NFQNL_COPY_PACKET, 0xffff) < 0) {
        std::cerr << "Error: nfq_set_mode() failed\n";
        nfq_destroy_queue(queueHandle);
        nfq_close(nfqHandle);
        return false;
    }

    fd = nfq_fd(nfqHandle);
    return true;
}

void PacketCapture::start() {
    running = true;
    captureThread = std::thread([this]() {
        char buf[4096] __attribute__ ((aligned));
        while (running) {
            int rv = recv(fd, buf, sizeof(buf), 0);
            if (rv >= 0) {
                nfq_handle_packet(nfqHandle, buf, rv);
            } else if (running) {
                std::cerr << "Error: recv() failed\n";
                break;
            }
        }
    });
}

void PacketCapture::stop() {
    running = false;
    if (captureThread.joinable())
        captureThread.join();
    if (queueHandle)
        nfq_destroy_queue(queueHandle);
    if (nfqHandle)
        nfq_close(nfqHandle);
}

int PacketCapture::internalCallback(struct nfq_q_handle* qh, struct nfgenmsg* nfmsg, struct nfq_data* nfa, void* data) {
    PacketCapture* self = static_cast<PacketCapture*>(data);
    if (self && self->userHandler)
        return self->userHandler(qh, nfmsg, nfa);
    return nfq_set_verdict(qh, 0, NF_ACCEPT, 0, nullptr);
}