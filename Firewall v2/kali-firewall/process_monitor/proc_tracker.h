#pragma once
#include <string>
#include <vector>

/**
 * @brief Struct representing a process-to-connection mapping.
 */
struct ProcConn {
    int pid = -1;
    std::string exe;         // Executable path
    std::string user;        // Username
    std::string local_addr;  // Local IP address
    int local_port = 0;      // Local port
    std::string remote_addr; // Remote IP address
    int remote_port = 0;     // Remote port
};

/**
 * @brief ProcTracker provides process-to-connection mapping for IPv4 and IPv6 TCP sockets.
 */
class ProcTracker {
public:
    /**
     * @brief List all active TCP connections with process info.
     * @return Vector of ProcConn entries.
     */
    static std::vector<ProcConn> listConnections();
};