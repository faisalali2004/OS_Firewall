#include "log_viewer.h"
#include "logger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include <QTimer>
#include <QComboBox>

LogViewer::LogViewer(Logger* logger, QWidget* parent)
    : QWidget(parent), logger(logger), currentPage(0), pageSize(50)
{
    auto* mainLayout = new QVBoxLayout(this);

    // --- Filter/Search Controls ---
    auto* filterLayout = new QHBoxLayout;
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Search IP, protocol, or action...");
    filterLayout->addWidget(new QLabel("Filter:"));
    filterLayout->addWidget(searchEdit);

    refreshBtn = new QPushButton("Refresh", this);
    clearBtn = new QPushButton("Clear Logs", this);
    filterLayout->addWidget(refreshBtn);
    filterLayout->addWidget(clearBtn);

    mainLayout->addLayout(filterLayout);

    // --- Table ---
    table = new QTableWidget(this);
    table->setColumnCount(8);
    table->setHorizontalHeaderLabels({"Time", "Src IP", "Src Port", "Dst IP", "Dst Port", "Protocol", "Action", "Info"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    mainLayout->addWidget(table);

    // --- Pagination ---
    auto* pageLayout = new QHBoxLayout;
    prevBtn = new QPushButton("Previous", this);
    nextBtn = new QPushButton("Next", this);
    pageLabel = new QLabel(this);
    pageLayout->addWidget(prevBtn);
    pageLayout->addWidget(pageLabel);
    pageLayout->addWidget(nextBtn);
    mainLayout->addLayout(pageLayout);

    setLayout(mainLayout);

    // --- Connections ---
    connect(refreshBtn, &QPushButton::clicked, this, &LogViewer::refreshLogs);
    connect(clearBtn, &QPushButton::clicked, this, &LogViewer::onClearLogs);
    connect(searchEdit, &QLineEdit::textChanged, this, &LogViewer::refreshLogs);
    connect(prevBtn, &QPushButton::clicked, this, &LogViewer::onPrevPage);
    connect(nextBtn, &QPushButton::clicked, this, &LogViewer::onNextPage);

    refreshLogs();
}

void LogViewer::refreshLogs() {
    table->setRowCount(0);
    QString filter = searchEdit->text().trimmed();
    std::vector<LogEntry> logs = logger->getLogs(pageSize, currentPage * pageSize);

    int row = 0;
    for (const auto& entry : logs) {
        // Filter
        if (!filter.isEmpty()) {
            QString all = QString::fromStdString(entry.timestamp + " " + entry.src_ip + " " + entry.dst_ip +
                                                 " " + entry.protocol + " " + entry.action + " " + entry.info);
            if (!all.contains(filter, Qt::CaseInsensitive))
                continue;
        }
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(entry.timestamp)));
        table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(entry.src_ip)));
        table->setItem(row, 2, new QTableWidgetItem(QString::number(entry.src_port)));
        table->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(entry.dst_ip)));
        table->setItem(row, 4, new QTableWidgetItem(QString::number(entry.dst_port)));
        table->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(entry.protocol)));
        auto* actionItem = new QTableWidgetItem(QString::fromStdString(entry.action));
        // Color code action
        if (entry.action == "allow")
            actionItem->setForeground(QBrush(Qt::darkGreen));
        else if (entry.action == "block")
            actionItem->setForeground(QBrush(Qt::red));
        else
            actionItem->setForeground(QBrush(Qt::blue));
        table->setItem(row, 6, actionItem);
        table->setItem(row, 7, new QTableWidgetItem(QString::fromStdString(entry.info)));
        ++row;
    }
    pageLabel->setText(QString("Page %1").arg(currentPage + 1));
    prevBtn->setEnabled(currentPage > 0);
    nextBtn->setEnabled(logs.size() == pageSize);
}

void LogViewer::onClearLogs() {
    if (QMessageBox::question(this, "Clear Logs", "Are you sure you want to clear all logs?") == QMessageBox::Yes) {
        logger->clearLogs();
        currentPage = 0;
        refreshLogs();
    }
}

void LogViewer::onPrevPage() {
    if (currentPage > 0) {
        --currentPage;
        refreshLogs();
    }
}

void LogViewer::onNextPage() {
    ++currentPage;
    refreshLogs();
}