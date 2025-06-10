#pragma once

#include <vector>
#include <string>

struct ProcConn {
    int pid;
    std::string exe;
    std::string user;
    std::string local_addr;
    std::string remote_addr;
    int local_port;
    int remote_port;
};

class ProcTracker {
public:
    // Returns a list of all TCP connections with process info
    std::vector<ProcConn> listConnections();
};