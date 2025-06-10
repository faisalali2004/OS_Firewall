#pragma once
#include <string>
#include <sqlite3.h>
#include <mutex>

class Logger {
public:
    Logger();
    ~Logger();

    bool initDB(const std::string& db_path);
    void logEvent(const std::string& timestamp, const std::string& src_ip, int src_port,
                  const std::string& dst_ip, int dst_port, const std::string& protocol,
                  const std::string& action, const std::string& info = "");

private:
    sqlite3* db;
    std::mutex mtx;
    bool initialized;
};