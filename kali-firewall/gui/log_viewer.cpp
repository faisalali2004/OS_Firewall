#include "log_viewer.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <sqlite3.h>
#include <QTableWidget>
#include <QTimer>
#include <vector>
#include <string>

LogViewer::LogViewer(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    table = new QTableWidget(this);
    table->setColumnCount(8);
    table->setHorizontalHeaderLabels({"Time", "Src IP", "Src Port", "Dst IP", "Dst Port", "Proto", "Action", "Info"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(table);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &LogViewer::refreshLogs);
    timer->start(2000); // Refresh every 2 seconds

    refreshLogs();
}

void LogViewer::refreshLogs() {
    loadLogs();
}

void LogViewer::loadLogs() {
    sqlite3* db = nullptr;
    if (sqlite3_open("../logs/firewall.db", &db) != SQLITE_OK) return;

    const char* sql = "SELECT timestamp, src_ip, src_port, dst_ip, dst_port, protocol, action, info FROM logs ORDER BY id DESC LIMIT 100";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return;
    }

    table->setRowCount(0);
    int row = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        table->insertRow(row);
        for (int col = 0; col < 8; ++col) {
            auto* item = new QTableWidgetItem(reinterpret_cast<const char*>(sqlite3_column_text(stmt, col)));
            table->setItem(row, col, item);
        }
        ++row;
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}