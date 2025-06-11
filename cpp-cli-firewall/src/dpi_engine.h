#pragma once
#include "packet.h"
#include <string>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <sstream>

class DPIEngine {
public:
    // Returns true if notable content is detected
    bool inspect(const Packet& pkt) const {
        // --- HTTP Inspection ---
        if (pkt.protocol == "TCP" && (pkt.dstPort == 80 || pkt.dstPort == 8080 || pkt.dstPort == 8000)) {
            std::string payload(pkt.data.begin() + 14 + 20 + 20, pkt.data.end()); // Ethernet+IP+TCP header skip (approx)
            if (payload.find("GET ") != std::string::npos || payload.find("POST ") != std::string::npos) {
                std::cout << "[DPI] HTTP request detected: ";
                printHttpSummary(payload);
                return true;
            }
        }

        // --- DNS Inspection ---
        if (pkt.protocol == "UDP" && (pkt.dstPort == 53 || pkt.srcPort == 53)) {
            std::string domain = extractDnsQuery(pkt);
            if (!domain.empty()) {
                std::cout << "[DPI] DNS Query detected: " << domain << std::endl;
                return true;
            }
        }

        // --- Suspicious Keywords ---
        std::string payload(pkt.data.begin(), pkt.data.end());
        std::string lower_payload = payload;
        std::transform(lower_payload.begin(), lower_payload.end(), lower_payload.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        if (lower_payload.find("malware") != std::string::npos ||
            lower_payload.find("attack") != std::string::npos) {
            std::cout << "[DPI] Suspicious keyword detected at timestamp " << pkt.timestamp << std::endl;
            return true;
        }

        // --- Large Payloads ---
        if (payload.size() > 1000) {
            std::cout << "[DPI] Large payload detected (" << payload.size() << " bytes) at timestamp " << pkt.timestamp << std::endl;
            return true;
        }

        return false;
    }

private:
    // Print a summary of the HTTP request (method, host, user-agent)
    void printHttpSummary(const std::string& payload) const {
        std::istringstream iss(payload);
        std::string line;
        std::string method, host, userAgent;
        if (std::getline(iss, line)) {
            method = line;
        }
        while (std::getline(iss, line)) {
            if (line.find("Host:") == 0) host = line.substr(6);
            if (line.find("User-Agent:") == 0) userAgent = line.substr(12);
        }
        std::cout << method;
        if (!host.empty()) std::cout << " Host:" << host;
        if (!userAgent.empty()) std::cout << " UA:" << userAgent;
        std::cout << std::endl;
    }

    // Extract DNS query domain from UDP payload
    std::string extractDnsQuery(const Packet& pkt) const {
        // DNS header is 12 bytes after UDP header
        size_t ethLen = 14;
        size_t ipLen = 20; // IPv4 header, no options
        size_t udpLen = 8;
        size_t dnsOffset = ethLen + ipLen + udpLen;
        if (pkt.data.size() <= dnsOffset + 12) return "";

        size_t qnameOffset = dnsOffset + 12;
        std::string domain;
        size_t i = qnameOffset;
        while (i < pkt.data.size() && pkt.data[i] != 0) {
            int len = pkt.data[i];
            if (len == 0 || i + len >= pkt.data.size()) break;
            if (!domain.empty()) domain += ".";
            domain += std::string(pkt.data.begin() + i + 1, pkt.data.begin() + i + 1 + len);
            i += len + 1;
        }
        return domain;
    }
};