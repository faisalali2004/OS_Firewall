#include "logger.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <filesystem>

Logger::Logger() : db(nullptr), initialized(false) {}

Logger::~Logger() {
    std::lock_guard<std::mutex> lock(mtx);
    if (db) sqlite3_close(db);
}

bool Logger::initDB(const std::string& db_path) {
    std::lock_guard<std::mutex> lock(mtx);

    // Optional: Log rotation if file > 10MB
    namespace fs = std::filesystem;
    try {
        if (fs::exists(db_path) && fs::file_size(db_path) > 10 * 1024 * 1024) {
            std::string backup = db_path + ".bak";
            fs::rename(db_path, backup);
            std::cout << "[Logger] Log DB rotated: " << backup << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "[Logger] Log rotation failed: " << e.what() << std::endl;
    }

    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        std::cerr << "[Logger] Failed to open SQLite DB: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
        initialized = false;
        return false;
    }
    const char* create_table =
        "CREATE TABLE IF NOT EXISTS logs ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "timestamp TEXT,"
        "src_ip TEXT,"
        "src_port INTEGER,"
        "dst_ip TEXT,"
        "dst_port INTEGER,"
        "protocol TEXT,"
        "action TEXT,"
        "info TEXT"
        ");";
    char* err = nullptr;
    if (sqlite3_exec(db, create_table, nullptr, nullptr, &err) != SQLITE_OK) {
        std::cerr << "[Logger] Failed to create table: " << (err ? err : "unknown error") << std::endl;
        if (err) sqlite3_free(err);
        sqlite3_close(db);
        db = nullptr;
        initialized = false;
        return false;
    }
    initialized = true;
    return true;
}

void Logger::logEvent(const std::string& timestamp, const std::string& src_ip, int src_port,
                      const std::string& dst_ip, int dst_port, const std::string& protocol,
                      const std::string& action, const std::string& info) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!initialized) {
        std::cerr << "[Logger] Not initialized, cannot log event." << std::endl;
        return;
    }
    std::string sql = "INSERT INTO logs (timestamp, src_ip, src_port, dst_ip, dst_port, protocol, action, info) VALUES (?,?,?,?,?,?,?,?);";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, timestamp.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, src_ip.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, src_port);
        sqlite3_bind_text(stmt, 4, dst_ip.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 5, dst_port);
        sqlite3_bind_text(stmt, 6, protocol.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 7, action.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 8, info.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "[Logger] Failed to insert log: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_finalize(stmt);
    } else {
        std::cerr << "[Logger] Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
    }

    // Print to console for debugging (with timestamp)
    std::cout << "[" << timestamp << "] " << src_ip << ":" << src_port << " -> "
              << dst_ip << ":" << dst_port << " " << protocol << " " << action
              << " " << info << std::endl;
}

std::vector<LogEntry> Logger::getLogs(int limit, int offset) {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<LogEntry> logs;
    if (!initialized) return logs;

    std::ostringstream oss;
    oss << "SELECT timestamp,src_ip,src_port,dst_ip,dst_port,protocol,action,info FROM logs "
        << "ORDER BY id DESC LIMIT " << limit << " OFFSET " << offset << ";";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, oss.str().c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            LogEntry entry;
            entry.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            entry.src_ip = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            entry.src_port = sqlite3_column_int(stmt, 2);
            entry.dst_ip = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            entry.dst_port = sqlite3_column_int(stmt, 4);
            entry.protocol = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            entry.action = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            entry.info = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            logs.push_back(entry);
        }
        sqlite3_finalize(stmt);
    }
    return logs;
}

void Logger::clearLogs() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!initialized) return;
    const char* sql = "DELETE FROM logs;";
    char* err = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err) != SQLITE_OK) {
        std::cerr << "[Logger] Failed to clear logs: " << (err ? err : "unknown error") << std::endl;
        if (err) sqlite3_free(err);
    }
}