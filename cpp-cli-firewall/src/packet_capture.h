#ifndef PACKET_CAPTURE_H
#define PACKET_CAPTURE_H

#include "packet.h"
#include <functional>
#include <string>

class PacketCapture {
public:
    PacketCapture() = default;
    ~PacketCapture() = default;
    void startCapture(const std::string& iface, std::function<void(const Packet&)> callback) {
        // Integrate with libpcap or similar to capture packets and call callback(pkt)
        // This is a stub for demonstration.
    }
};

#endif // PACKET_CAPTURE_H