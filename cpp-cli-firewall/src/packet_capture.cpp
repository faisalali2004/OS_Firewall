#include "packet_capture.h"
#include <iostream>
#include <cstring>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

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

                // Parse Ethernet, IPv4, TCP/UDP headers
                if (header->len > 14) {
                    const uint8_t* ipData = data + 14;
                    struct ip* iph = (struct ip*)ipData;
                    if (iph->ip_v == 4) {
                        pkt.srcIP = inet_ntoa(iph->ip_src);
                        pkt.dstIP = inet_ntoa(iph->ip_dst);
                        // TCP or UDP
                        if (iph->ip_p == IPPROTO_TCP && header->len > 14 + iph->ip_hl*4 + 4) {
                            struct tcphdr* tcph = (struct tcphdr*)(ipData + iph->ip_hl*4);
                            pkt.srcPort = ntohs(tcph->th_sport);
                            pkt.dstPort = ntohs(tcph->th_dport);
                            pkt.protocol = "TCP";
                        } else if (iph->ip_p == IPPROTO_UDP && header->len > 14 + iph->ip_hl*4 + 4) {
                            struct udphdr* udph = (struct udphdr*)(ipData + iph->ip_hl*4);
                            pkt.srcPort = ntohs(udph->uh_sport);
                            pkt.dstPort = ntohs(udph->uh_dport);
                            pkt.protocol = "UDP";
                        } else {
                            pkt.protocol = std::to_string(iph->ip_p);
                        }
                    }
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