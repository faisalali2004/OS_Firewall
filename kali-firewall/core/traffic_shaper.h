#pragma once

#include <string>

class TrafficShaper {
public:
    // Shape bandwidth on a given interface (rate in kbit/s)
    bool shape(const std::string& iface, int rate_kbit, int burst_kbit = 32, int latency_ms = 400);
    // Remove shaping from an interface
    bool clear(const std::string& iface);
};