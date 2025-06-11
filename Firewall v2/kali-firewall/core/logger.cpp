#include "logger.h"
#include <iostream>
#include <sstream>

Logger::Logger()
    : db(nullptr), initialized(false)
{}

Logger::~Logger() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

bool Logger::initDB(const std::string& db_path) {
    std::lock_guard<std::mutex> lock(mtx);
    if (initialized) return true;

    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        std::cerr << "[Logger] Failed to open DB: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
        return false;
    }

    const char* createTableSQL =
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
    char* errMsg = nullptr;
    if (sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "[Logger] Failed to create table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    initialized = true;
    return true;
}

void Logger::logEvent(const std::string& timestamp, const std::string& src_ip, int src_port,
                      const std::string& dst_ip, int dst_port, const std::string& protocol,
                      const std::string& action, const std::string& info)
{
    std::lock_guard<std::mutex> lock(mtx);
    if (!initialized || !db) return;

    const char* insertSQL =
        "INSERT INTO logs (timestamp, src_ip, src_port, dst_ip, dst_port, protocol, action, info) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[Logger] Failed to prepare insert: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

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
}

std::vector<LogEntry> Logger::getLogs(int limit, int offset) {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<LogEntry> result;
    if (!initialized || !db) return result;

    std::ostringstream oss;
    oss << "SELECT timestamp, src_ip, src_port, dst_ip, dst_port, protocol, action, info "
        << "FROM logs ORDER BY id DESC LIMIT " << limit << " OFFSET " << offset << ";";
    std::string query = oss.str();

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[Logger] Failed to prepare select: " << sqlite3_errmsg(db) << std::endl;
        return result;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        LogEntry entry;
        entry.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        entry.src_ip    = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        entry.src_port  = sqlite3_column_int(stmt, 2);
        entry.dst_ip    = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        entry.dst_port  = sqlite3_column_int(stmt, 4);
        entry.protocol  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        entry.action    = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        entry.info      = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        result.push_back(entry);
    }
    sqlite3_finalize(stmt);
    return result;
}

void Logger::clearLogs() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!initialized || !db) return;

    const char* clearSQL = "DELETE FROM logs;";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, clearSQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "[Logger] Failed to clear logs: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}