#ifndef PACKET_H
#define PACKET_H

#include <string>
#include <cstdint>

struct Packet {
    enum class Direction { IN, OUT };
    std::string srcIP;
    std::string dstIP;
    uint16_t srcPort;
    uint16_t dstPort;
    std::string protocol; // e.g., "TCP", "UDP", "ICMP"
    size_t size;
    Direction direction;

    Packet() : srcPort(0), dstPort(0), size(0), direction(Direction::IN) {}
};

#endif // PACKET_H