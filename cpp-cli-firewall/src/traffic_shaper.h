#pragma once
#include "packet.h"
#include <chrono>
#include <mutex>

class TrafficShaper {
public:
    TrafficShaper(size_t rateBytesPerSec = 1024 * 1024) // Default: 1MB/s
        : rate(rateBytesPerSec), tokens(rateBytesPerSec), lastRefill(std::chrono::steady_clock::now()) {}

    // Call this before forwarding/processing a packet
    void shape(const Packet& pkt) {
        std::unique_lock<std::mutex> lock(mtx);
        refill();
        size_t pktSize = pkt.length;
        while (tokens < pktSize) {
            // Not enough tokens, sleep for a short while
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            lock.lock();
            refill();
        }
        tokens -= pktSize;
        // Optionally log shaping event
        // std::cout << "[Shaper] Allowed packet of size " << pktSize << " bytes\n";
    }

    void setRate(size_t newRate) {
        std::lock_guard<std::mutex> lock(mtx);
        rate = newRate;
        tokens = std::min(tokens, rate);
    }

private:
    void refill() {
        auto now = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastRefill).count();
        if (ms > 0) {
            size_t add = (rate * ms) / 1000;
            tokens = std::min(tokens + add, rate);
            lastRefill = now;
        }
    }

    size_t rate; // bytes per second
    size_t tokens;
    std::chrono::steady_clock::time_point lastRefill;
    std::mutex mtx;
};