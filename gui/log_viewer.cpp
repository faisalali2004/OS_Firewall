#include "log_viewer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>

LogViewer::LogViewer(QWidget* parent)
    : QWidget(parent),
      table(new QTableWidget(this)),
      timer(new QTimer(this)),
      refreshBtn(new QPushButton("Refresh Now", this)),
      statusLabel(new QLabel(this)),
      logPath("../logs/firewall_log.json")
{
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"Timestamp", "Source IP", "Destination IP", "Protocol", "Action"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto* btnLayout = new QHBoxLayout;
    btnLayout->addWidget(refreshBtn);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(table);
    mainLayout->addLayout(btnLayout);
    mainLayout->addWidget(statusLabel);

    setLayout(mainLayout);

    connect(timer, &QTimer::timeout, this, &LogViewer::refreshLogs);
    connect(refreshBtn, &QPushButton::clicked, this, &LogViewer::manualRefresh);

    timer->start(3000); // Refresh every 3 seconds
    refreshLogs();
    updateStatus("Ready.");
}

LogViewer::~LogViewer() = default;

void LogViewer::manualRefresh() {
    refreshLogs();
    updateStatus("Manually refreshed logs.");
}

void LogViewer::refreshLogs() {
    loadLogs();
}

void LogViewer::loadLogs() {
    QFile file(logPath);
    if (!file.open(QIODevice::ReadOnly)) {
        updateStatus("Failed to open log file: " + logPath, true);
        table->setRowCount(0);
        return;
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        updateStatus("Failed to parse log JSON: " + err.errorString(), true);
        table->setRowCount(0);
        return;
    }
    if (!doc.isArray()) {
        updateStatus("Log file format error.", true);
        table->setRowCount(0);
        return;
    }

    QJsonArray logs = doc.array();
    table->setRowCount(0);
    for (const auto& val : logs) {
        if (!val.isObject()) continue;
        QJsonObject obj = val.toObject();
        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(obj.value("timestamp").toString()));
        table->setItem(row, 1, new QTableWidgetItem(obj.value("src_ip").toString()));
        table->setItem(row, 2, new QTableWidgetItem(obj.value("dst_ip").toString()));
        table->setItem(row, 3, new QTableWidgetItem(obj.value("protocol").toString()));
        table->setItem(row, 4, new QTableWidgetItem(obj.value("action").toString()));
    }
    updateStatus(QString("Loaded %1 log entries.").arg(table->rowCount()));
}

void LogViewer::updateStatus(const QString& msg, bool error) {
    statusLabel->setText(msg);
    QPalette pal = statusLabel->palette();
    pal.setColor(QPalette::WindowText, error ? Qt::red : Qt::darkGreen);
    statusLabel->setPalette(pal);
}