#pragma once
#include <thread>
#include <atomic>
#include <functional>
#include <pcap.h>
#include <string>
#include <mutex>
#include "packet.h"

class PacketCapture {
public:
    PacketCapture();
    ~PacketCapture();

    // Start capturing packets on the given interface, calling callback for each packet
    void startCapture(const std::string& iface, std::function<void(const Packet&)> callback);

    // Stop capturing packets
    void stopCapture();

    // Returns true if capture is running
    bool isRunning() const;

    // Get last error message (if any)
    std::string getLastError() const;

private:
    void captureLoop(const std::string& iface, std::function<void(const Packet&)> callback);

    std::thread captureThread;
    std::atomic<bool> running{false};
    pcap_t* handle{nullptr};
    mutable std::mutex errorMutex;
    std::string lastError;
};