#include "traffic_shaper.h"
#include <cstdlib>
#include <sstream>

bool TrafficShaper::shape(const std::string& iface, int rate_kbit, int burst_kbit, int latency_ms) {
    std::ostringstream cmd;
    cmd << "tc qdisc replace dev " << iface
        << " root tbf rate " << rate_kbit << "kbit"
        << " burst " << burst_kbit << "kbit"
        << " latency " << latency_ms << "ms";
    return std::system(cmd.str().c_str()) == 0;
}

bool TrafficShaper::clear(const std::string& iface) {
    std::ostringstream cmd;
    cmd << "tc qdisc del dev " << iface << " root";
    return std::system(cmd.str().c_str()) == 0;
}