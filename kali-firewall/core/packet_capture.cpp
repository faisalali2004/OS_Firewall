#include "packet_capture.h"
#include "logger.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <atomic>
#include <csignal>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <ctime>
#include <linux/netfilter.h> // For NF_DROP, NF_ACCEPT
#include <sys/resource.h>    // For getrusage

namespace {
constexpr size_t DEFAULT_BUF_SIZE = 0x10000; // 64KB
}

static std::string protoName(uint8_t proto) {
    switch (proto) {
        case IPPROTO_TCP: return "TCP";
        case IPPROTO_UDP: return "UDP";
        case IPPROTO_ICMP: return "ICMP";
        default: return std::to_string(proto);
    }
}

PacketCapture::PacketCapture()
    : nfqHandle(nullptr), queueHandle(nullptr), fd(-1), running(false), ruleEngine(nullptr), dpiEngine(nullptr),
      totalPackets(0), blockedPackets(0) {}

PacketCapture::~PacketCapture() {
    stop();
}

bool PacketCapture::init(uint16_t queue_num, size_t buf_size) {
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

int PacketCapture::getCurrentMemoryUsageKB() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss;
}

int PacketCapture::internalCallback(struct nfq_q_handle* qh, struct nfgenmsg*, struct nfq_data* nfa, void* data) {
    PacketCapture* self = static_cast<PacketCapture*>(data);

    uint32_t id = 0;
    struct nfqnl_msg_packet_hdr* ph = nfq_get_msg_packet_hdr(nfa);
    if (ph) id = ntohl(ph->packet_id);

    unsigned char* pktData = nullptr;
    int len = nfq_get_payload(nfa, &pktData);

    std::string src_ip, dst_ip, protocol, info;
    int src_port = 0, dst_port = 0;
    std::string action = "allow"; // Default to allow

    if (len > 0 && pktData) {
        struct iphdr* iph = (struct iphdr*)pktData;
        char src[INET_ADDRSTRLEN], dst[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &iph->saddr, src, sizeof(src));
        inet_ntop(AF_INET, &iph->daddr, dst, sizeof(dst));
        src_ip = src;
        dst_ip = dst;
        protocol = protoName(iph->protocol);

        if (iph->protocol == IPPROTO_TCP && len >= (int)(iph->ihl*4 + sizeof(tcphdr))) {
            struct tcphdr* tcph = (struct tcphdr*)(pktData + iph->ihl*4);
            src_port = ntohs(tcph->source);
            dst_port = ntohs(tcph->dest);
        } else if (iph->protocol == IPPROTO_UDP && len >= (int)(iph->ihl*4 + sizeof(udphdr))) {
            struct udphdr* udph = (struct udphdr*)(pktData + iph->ihl*4);
            src_port = ntohs(udph->source);
            dst_port = ntohs(udph->dest);
        }
    }

    // --- RuleEngine and DPIEngine integration ---
    bool shouldBlock = false;
    if (self) {
        if (self->ruleEngine && self->ruleEngine->shouldBlock(src_ip, dst_ip, src_port, dst_port, protocol, pktData, len)) {
            shouldBlock = true;
            info = "Blocked by RuleEngine";
        }
        // DPI check only if not already blocked
        else if (self->dpiEngine && self->dpiEngine->shouldBlock(src_ip, dst_ip, src_port, dst_port, protocol, pktData, len)) {
            shouldBlock = true;
            info = "Blocked by DPIEngine";
        }
    }

    action = shouldBlock ? "block" : "allow";

    // Print to terminal for debug
    std::cout << "[PACKET] " << src_ip << ":" << src_port << " -> "
              << dst_ip << ":" << dst_port << " proto: " << protocol
              << " action: " << action << " info: " << info << std::endl;

    // Log to database (for LogViewer)
    std::time_t now = std::time(nullptr);
    char timebuf[32];
    std::strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    Logger::instance().logEvent(
        timebuf,
        src_ip,
        src_port,
        dst_ip,
        dst_port,
        protocol,
        action,
        info
    );

    // --- Memory and stats update ---
    if (self) {
        self->totalPackets++;
        if (shouldBlock) self->blockedPackets++;
        emit self->statsUpdated(self->totalPackets, self->blockedPackets, self->getCurrentMemoryUsageKB());
    }

    if (shouldBlock) {
        return nfq_set_verdict(qh, id, NF_DROP, 0, nullptr);
    } else {
        return nfq_set_verdict(qh, id, NF_ACCEPT, 0, nullptr);
    }
}