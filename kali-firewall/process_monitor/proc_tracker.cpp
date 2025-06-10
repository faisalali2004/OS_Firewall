#include "proc_tracker.h"
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <cstring>
#include <arpa/inet.h>

static std::string hexToIp(const std::string& hex) {
    unsigned int ip;
    sscanf(hex.c_str(), "%X", &ip);
    struct in_addr in;
    in.s_addr = htonl(ip);
    return std::string(inet_ntoa(in));
}

static int hexToPort(const std::string& hex) {
    int port;
    sscanf(hex.c_str(), "%X", &port);
    return port;
}

std::vector<ProcConn> ProcTracker::listConnections() {
    std::vector<ProcConn> result;
    std::ifstream f("/proc/net/tcp");
    std::string line;
    getline(f, line); // skip header
    while (getline(f, line)) {
        std::istringstream iss(line);
        std::string sl, local, rem, st, tx, rx, tr, tm, retr, uid, timeout, inode;
        iss >> sl >> local >> rem >> st >> tx >> rx >> tr >> tm >> retr >> uid >> timeout >> inode;
        size_t colon1 = local.find(':');
        size_t colon2 = rem.find(':');
        if (colon1 == std::string::npos || colon2 == std::string::npos) continue;
        std::string local_ip = hexToIp(local.substr(0, colon1));
        int local_port = hexToPort(local.substr(colon1 + 1));
        std::string rem_ip = hexToIp(rem.substr(0, colon2));
        int rem_port = hexToPort(rem.substr(colon2 + 1));
        int uid_num = std::stoi(uid);

        // Find PID by inode
        DIR* proc = opendir("/proc");
        if (!proc) continue;
        struct dirent* ent;
        while ((ent = readdir(proc))) {
            if (!isdigit(ent->d_name[0])) continue;
            std::string fd_dir = std::string("/proc/") + ent->d_name + "/fd";
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
                    if (slink.find("socket:[") != std::string::npos && slink.find(inode) != std::string::npos) {
                        ProcConn pc;
                        pc.pid = std::stoi(ent->d_name);
                        char exe_path[256] = {0};
                        std::string exe_link = std::string("/proc/") + ent->d_name + "/exe";
                        ssize_t exe_len = readlink(exe_link.c_str(), exe_path, sizeof(exe_path) - 1);
                        pc.exe = exe_len > 0 ? std::string(exe_path, exe_len) : "";
                        struct passwd* pw = getpwuid(uid_num);
                        pc.user = pw ? pw->pw_name : "";
                        pc.local_addr = local_ip;
                        pc.local_port = local_port;
                        pc.remote_addr = rem_ip;
                        pc.remote_port = rem_port;
                        result.push_back(pc);
                        break;
                    }
                }
            }
            closedir(fd);
        }
        closedir(proc);
    }
    return result;
}