#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <sqlite3.h>

struct LogEntry {
    std::string timestamp;
    std::string src_ip;
    int src_port;
    std::string dst_ip;
    int dst_port;
    std::string protocol;
    std::string action;
    std::string info;
};

class Logger {
public:
    Logger();
    ~Logger();

    // Singleton instance accessor
    static Logger& instance();

    bool initDB(const std::string& db_path);
    void logEvent(const std::string& timestamp, const std::string& src_ip, int src_port,
                  const std::string& dst_ip, int dst_port, const std::string& protocol,
                  const std::string& action, const std::string& info);

    std::vector<LogEntry> getLogs(int limit = 100, int offset = 0);
    void clearLogs();

private:
    sqlite3* db;
    bool initialized;
    std::mutex mtx;
};