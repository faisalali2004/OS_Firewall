#include "packet_capture.h"
#include <iostream>
#include <cstring>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <pcap/pcap.h>

#ifndef DLT_LINUX_SLL
#define DLT_LINUX_SLL 113
#endif
#ifndef DLT_NULL
#define DLT_NULL 0
#endif

PacketCapture::PacketCapture() : running(false), handle(nullptr) {}

PacketCapture::~PacketCapture() {
    stopCapture();
}

void PacketCapture::startCapture(const std::string& iface, std::function<void(const Packet&)> callback) {
    if (running) return;
    running = true;
    {
        std::lock_guard<std::mutex> lock(errorMutex);
        lastError.clear();
    }
    captureThread = std::thread([this, iface, callback]() {
        char errbuf[PCAP_ERRBUF_SIZE];
        pcap_t* pcap_handle = pcap_open_live(iface.c_str(), BUFSIZ, 1, 1000, errbuf);
        if (!pcap_handle) {
            std::lock_guard<std::mutex> lock(errorMutex);
            lastError = errbuf;
            std::cerr << "[!] pcap_open_live failed: " << errbuf << std::endl;
            running = false;
            return;
        }
        handle = pcap_handle;
        int dlt = pcap_datalink(pcap_handle);

        while (running) {
            struct pcap_pkthdr* header = nullptr;
            const u_char* data = nullptr;
            int ret = pcap_next_ex(pcap_handle, &header, &data);
            if (ret == 1 && header && data) {
                Packet pkt;
                pkt.timestamp = header->ts.tv_sec;
                pkt.length = header->len;
                pkt.data.assign(data, data + header->len);
                pkt.size = header->len;
                pkt.srcPort = 0;
                pkt.dstPort = 0;
                pkt.protocol = "";
                pkt.srcIP = "";
                pkt.dstIP = "";

                // --- Layer 2 offset detection ---
                size_t l2_offset = 0;
                if (dlt == DLT_EN10MB) {
                    l2_offset = sizeof(struct ether_header);
                } else if (dlt == DLT_LINUX_SLL) {
                    l2_offset = 16;
                } else if (dlt == DLT_NULL) {
                    l2_offset = 4;
                } else {
                    pkt.protocol = "UNKNOWN-L2";
                    callback(pkt);
                    continue;
                }

                // --- IPv4 parsing ---
                if (header->len >= l2_offset + sizeof(struct ip)) {
                    const struct ip* iph = (const struct ip*)(data + l2_offset);
                    if (iph->ip_v == 4) {
                        char srcbuf[INET_ADDRSTRLEN] = {0};
                        char dstbuf[INET_ADDRSTRLEN] = {0};
                        inet_ntop(AF_INET, &(iph->ip_src), srcbuf, INET_ADDRSTRLEN);
                        inet_ntop(AF_INET, &(iph->ip_dst), dstbuf, INET_ADDRSTRLEN);
                        pkt.srcIP = srcbuf;
                        pkt.dstIP = dstbuf;

                        // TCP or UDP
                        if (iph->ip_p == IPPROTO_TCP && header->len >= l2_offset + iph->ip_hl*4 + sizeof(struct tcphdr)) {
                            const struct tcphdr* tcph = (const struct tcphdr*)((const uint8_t*)iph + iph->ip_hl*4);
                            pkt.srcPort = ntohs(tcph->th_sport);
                            pkt.dstPort = ntohs(tcph->th_dport);
                            pkt.protocol = "TCP";
                        } else if (iph->ip_p == IPPROTO_UDP && header->len >= l2_offset + iph->ip_hl*4 + sizeof(struct udphdr)) {
                            const struct udphdr* udph = (const struct udphdr*)((const uint8_t*)iph + iph->ip_hl*4);
                            pkt.srcPort = ntohs(udph->uh_sport);
                            pkt.dstPort = ntohs(udph->uh_dport);
                            pkt.protocol = "UDP";
                        } else {
                            pkt.protocol = std::to_string(iph->ip_p);
                        }
                    } else {
                        pkt.protocol = "NON-IPv4";
                    }
                } else {
                    pkt.protocol = "TOO-SHORT";
                }

                try {
                    callback(pkt);
                } catch (const std::exception& ex) {
                    std::cerr << "[!] Exception in packet callback: " << ex.what() << std::endl;
                }
            } else if (ret == -1) {
                std::lock_guard<std::mutex> lock(errorMutex);
                lastError = pcap_geterr(pcap_handle);
                std::cerr << "[!] pcap_next_ex error: " << lastError << std::endl;
                break;
            } else if (ret == -2) {
                // pcap_breakloop called
                break;
            }
        }
        pcap_close(pcap_handle);
        handle = nullptr;
        running = false;
    });
}

void PacketCapture::stopCapture() {
    running = false;
    if (handle) {
        pcap_breakloop(handle);
    }
    if (captureThread.joinable()) {
        captureThread.join();
    }
    if (handle) {
        pcap_close(handle);
        handle = nullptr;
    }
}

bool PacketCapture::isRunning() const {
    return running;
}

std::string PacketCapture::getLastError() const {
    std::lock_guard<std::mutex> lock(errorMutex);
    return lastError;
}