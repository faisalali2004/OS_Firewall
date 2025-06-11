#include "proc_tracker.h"
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <cstring>
#include <arpa/inet.h>
#include <set>
#include <algorithm>
#include <map>
#include <set>

// Convert hex IP to dotted-decimal (IPv4)
static std::string hexToIp(const std::string& hex) {
    unsigned int ip;
    sscanf(hex.c_str(), "%X", &ip);
    struct in_addr in;
    in.s_addr = htonl(ip);
    return std::string(inet_ntoa(in));
}

// Convert hex port to int
static int hexToPort(const std::string& hex) {
    int port;
    sscanf(hex.c_str(), "%X", &port);
    return port;
}

// Convert hex IPv6 to string
static std::string hexToIp6(const std::string& hex) {
    struct in6_addr addr6;
    for (int i = 0; i < 16; ++i) {
        std::string byteStr = hex.substr(i * 2, 2);
        addr6.s6_addr[i] = (uint8_t)strtol(byteStr.c_str(), nullptr, 16);
    }
    char buf[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &addr6, buf, sizeof(buf));
    return std::string(buf);
}

// Helper: Map inode to (pid, exe, user)
static void buildInodeMap(std::map<std::string, ProcConn>& inodeMap) {
    DIR* proc = opendir("/proc");
    if (!proc) return;
    struct dirent* ent;
    while ((ent = readdir(proc))) {
        if (!isdigit(ent->d_name[0])) continue;
        std::string pidStr = ent->d_name;
        int pid = std::stoi(pidStr);
        std::string fd_dir = "/proc/" + pidStr + "/fd";
        DIR* fd = opendir(fd_dir.c_str());
        if (!fd) continue;
        struct dirent* fdent;
        while ((fdent = readdir(fd))) {
            if (fdent->d_name[0] == '.') continue;
            char link[256];
            std::string fd_path = fd_dir + "/" + fdent->d_name;
            ssize_t len = readlink(fd_path.c_str(), link, sizeof(link) - 1);
            if (len > 0) {
                link[len] = '\0';
                std::string slink(link);
                size_t start = slink.find("socket:[");
                if (start != std::string::npos) {
                    size_t end = slink.find("]", start);
                    if (end != std::string::npos) {
                        std::string inode = slink.substr(start + 8, end - (start + 8));
                        ProcConn pc;
                        pc.pid = pid;
                        char exe_path[256] = {0};
                        std::string exe_link = "/proc/" + pidStr + "/exe";
                        ssize_t exe_len = readlink(exe_link.c_str(), exe_path, sizeof(exe_path) - 1);
                        pc.exe = exe_len > 0 ? std::string(exe_path, exe_len) : "";
                        struct stat st;
                        if (stat(fd_path.c_str(), &st) == 0) {
                            struct passwd* pw = getpwuid(st.st_uid);
                            pc.user = pw ? pw->pw_name : "";
                        }
                        inodeMap[inode] = pc;
                    }
                }
            }
        }
        closedir(fd);
    }
    closedir(proc);
}

// Parse /proc/net/tcp or tcp6
static void parseTcpFile(const std::string& path, bool ipv6, std::vector<ProcConn>& result, const std::map<std::string, ProcConn>& inodeMap, std::set<std::string>& seen) {
    std::ifstream f(path);
    std::string line;
    getline(f, line); // skip header
    while (getline(f, line)) {
        std::istringstream iss(line);
        std::string sl, local, rem, st, tx, rx, tr, tm, retr, uid, timeout, inode;
        iss >> sl >> local >> rem >> st >> tx >> rx >> tr >> tm >> retr >> uid >> timeout >> inode;
        if (inode.empty() || seen.count(inode)) continue;
        seen.insert(inode);

        size_t colon1 = local.find(':');
        size_t colon2 = rem.find(':');
        if (colon1 == std::string::npos || colon2 == std::string::npos) continue;

        std::string local_ip = ipv6 ? hexToIp6(local.substr(0, colon1)) : hexToIp(local.substr(0, colon1));
        int local_port = hexToPort(local.substr(colon1 + 1));
        std::string rem_ip = ipv6 ? hexToIp6(rem.substr(0, colon2)) : hexToIp(rem.substr(0, colon2));
        int rem_port = hexToPort(rem.substr(colon2 + 1));
        int uid_num = std::stoi(uid);

        auto it = inodeMap.find(inode);
        if (it != inodeMap.end()) {
            ProcConn pc = it->second;
            pc.local_addr = local_ip;
            pc.local_port = local_port;
            pc.remote_addr = rem_ip;
            pc.remote_port = rem_port;
            result.push_back(pc);
        }
    }
}

std::vector<ProcConn> ProcTracker::listConnections() {
    std::vector<ProcConn> result;
    std::map<std::string, ProcConn> inodeMap;
    std::set<std::string> seen;
    buildInodeMap(inodeMap);
    parseTcpFile("/proc/net/tcp", false, result, inodeMap, seen);
    parseTcpFile("/proc/net/tcp6", true, result, inodeMap, seen);
    return result;
}