#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

// Packet direction (optional, can be used for further analysis)
enum class Direction { IN, OUT };

struct Packet {
    uint64_t timestamp;                // Unix timestamp (seconds)
    uint32_t length;                   // Packet length in bytes
    std::vector<uint8_t> data;         // Raw packet data

    // Parsed fields (set by packet capture logic)
    std::string srcIP;
    std::string dstIP;
    uint16_t srcPort = 0;
    uint16_t dstPort = 0;
    std::string protocol;
    Direction direction = Direction::IN;
    size_t size = 0;

    // Return a hex string representation of the packet data (first N bytes)
    std::string toHex(size_t maxBytes = 32) const {
        std::ostringstream oss;
        size_t n = std::min(maxBytes, data.size());
        for (size_t i = 0; i < n; ++i) {
            oss << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(data[i]) << " ";
        }
        if (data.size() > maxBytes) oss << "...";
        return oss.str();
    }

    // Return a printable ASCII string of the payload (non-printables as '.')
    std::string toAscii(size_t maxBytes = 32) const {
        std::ostringstream oss;
        size_t n = std::min(maxBytes, data.size());
        for (size_t i = 0; i < n; ++i) {
            char c = static_cast<char>(data[i]);
            oss << (std::isprint(static_cast<unsigned char>(c)) ? c : '.');
        }
        if (data.size() > maxBytes) oss << "...";
        return oss.str();
    }

    // Return a summary string for logging or display
    std::string summary() const {
        std::ostringstream oss;
        oss << "[Packet] ts=" << timestamp
            << " len=" << length
            << " src=" << srcIP << ":" << srcPort
            << " dst=" << dstIP << ":" << dstPort
            << " proto=" << protocol
            << " hex=" << toHex(16)
            << " ascii=\"" << toAscii(16) << "\"";
        return oss.str();
    }
};