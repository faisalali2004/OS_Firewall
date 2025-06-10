#include "dashboard.h"
#include <QVBoxLayout>
#include <QHeaderView>

Dashboard::Dashboard(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);

    packetRateLabel = new QLabel("Packet Rate: -- pkts/sec", this);
    dpiMatchLabel = new QLabel("DPI Matches: --", this);

    connTable = new QTableWidget(this);
    connTable->setColumnCount(5);
    connTable->setHorizontalHeaderLabels({"PID", "User", "Process", "Src -> Dst", "Protocol"});
    connTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    layout->addWidget(packetRateLabel);
    layout->addWidget(dpiMatchLabel);
    layout->addWidget(new QLabel("Active Connections:", this));
    layout->addWidget(connTable);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Dashboard::refreshStats);
    timer->start(2000); // Refresh every 2 seconds

    refreshStats();
}

void Dashboard::refreshStats() {
    // Placeholder: Replace with real stats from your backend
    packetRateLabel->setText("Packet Rate: 123 pkts/sec");
    dpiMatchLabel->setText("DPI Matches: HTTP(5), DNS(2)");

    loadConnections();
}

void Dashboard::loadConnections() {
    // Placeholder: Replace with real connection data from ProcTracker
    connTable->setRowCount(0);
    int row = connTable->rowCount();
    connTable->insertRow(row);
    connTable->setItem(row, 0, new QTableWidgetItem("1234"));
    connTable->setItem(row, 1, new QTableWidgetItem("root"));
    connTable->setItem(row, 2, new QTableWidgetItem("/usr/bin/python3"));
    connTable->setItem(row, 3, new QTableWidgetItem("192.168.1.2:12345 -> 8.8.8.8:53"));
    connTable->setItem(row, 4, new QTableWidgetItem("UDP"));
}