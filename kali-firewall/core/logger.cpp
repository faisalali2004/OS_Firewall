#include "logger.h"
#include <iostream>

Logger::Logger() : db(nullptr), initialized(false) {}

Logger::~Logger() {
    if (db) sqlite3_close(db);
}

bool Logger::initDB(const std::string& db_path) {
    std::lock_guard<std::mutex> lock(mtx);
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Failed to open SQLite DB: " << sqlite3_errmsg(db) << std::endl;
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
        std::cerr << "Failed to create table: " << err << std::endl;
        sqlite3_free(err);
        return false;
    }
    initialized = true;
    return true;
}

void Logger::logEvent(const std::string& timestamp, const std::string& src_ip, int src_port,
                      const std::string& dst_ip, int dst_port, const std::string& protocol,
                      const std::string& action, const std::string& info) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!initialized) return;
    std::string sql = "INSERT INTO logs (timestamp, src_ip, src_port, dst_ip, dst_port, protocol, action, info) VALUES (?,?,?,?,?,?,?,?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, timestamp.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, src_ip.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, src_port);
        sqlite3_bind_text(stmt, 4, dst_ip.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 5, dst_port);
        sqlite3_bind_text(stmt, 6, protocol.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 7, action.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 8, info.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    // Optional: also print to console for debugging
    std::cout << "[" << timestamp << "] " << src_ip << ":" << src_port << " -> "
              << dst_ip << ":" << dst_port << " " << protocol << " " << action
              << " " << info << std::endl;
}