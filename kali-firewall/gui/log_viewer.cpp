#include "log_viewer.h"
#include "logger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QBrush>
#include <QColor>
#include <QFont>
#include <algorithm>

LogViewer::LogViewer(QWidget* parent)
    : QWidget(parent), currentPage(0), pageSize(50)
{
    auto* mainLayout = new QVBoxLayout(this);

    // Table for logs
    table = new QTableWidget(this);
    table->setColumnCount(8);
    table->setHorizontalHeaderLabels({"Time", "Src IP", "Src Port", "Dst IP", "Dst Port", "Protocol", "Action", "Info"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSortingEnabled(true);
    table->setAlternatingRowColors(true);

    // Pagination controls
    auto* navLayout = new QHBoxLayout;
    prevBtn = new QPushButton("Previous", this);
    nextBtn = new QPushButton("Next", this);
    pageLabel = new QLabel(this);
    navLayout->addWidget(prevBtn);
    navLayout->addWidget(pageLabel);
    navLayout->addWidget(nextBtn);

    // Clear logs button
    clearBtn = new QPushButton("Clear Logs", this);

    mainLayout->addWidget(table);
    mainLayout->addLayout(navLayout);
    mainLayout->addWidget(clearBtn);

    setLayout(mainLayout);

    connect(prevBtn, &QPushButton::clicked, this, &LogViewer::onPrevPage);
    connect(nextBtn, &QPushButton::clicked, this, &LogViewer::onNextPage);
    connect(clearBtn, &QPushButton::clicked, this, &LogViewer::onClearLogs);

    refreshLogs();
}

void LogViewer::refreshLogs() {
    int totalLogs = static_cast<int>(Logger::instance().getLogs().size());
    int totalPages = (totalLogs + pageSize - 1) / pageSize;
    if (totalPages == 0) totalPages = 1;
    if (currentPage >= totalPages) currentPage = totalPages - 1;

    logs = Logger::instance().getLogs(currentPage * pageSize, pageSize);

    table->clearContents();
    table->setRowCount(static_cast<int>(logs.size()));

    for (int row = 0; row < static_cast<int>(logs.size()); ++row) {
        const auto& entry = logs[row];
        table->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(entry.timestamp)));
        table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(entry.src_ip)));
        table->setItem(row, 2, new QTableWidgetItem(QString::number(entry.src_port)));
        table->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(entry.dst_ip)));
        table->setItem(row, 4, new QTableWidgetItem(QString::number(entry.dst_port)));
        table->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(entry.protocol)));
        table->setItem(row, 6, new QTableWidgetItem(QString::fromStdString(entry.action)));
        table->setItem(row, 7, new QTableWidgetItem(QString::fromStdString(entry.info)));

        // Highlight recent entries (first page only)
        if (currentPage == 0 && row < 5) {
            QBrush highlight(QColor(230, 255, 230));
            QFont boldFont;
            boldFont.setBold(true);
            for (int col = 0; col < 8; ++col) {
                table->item(row, col)->setBackground(highlight);
                table->item(row, col)->setFont(boldFont);
            }
        }
    }

    // If no logs, show a placeholder row
    if (logs.empty()) {
        table->setRowCount(1);
        for (int col = 0; col < 8; ++col) {
            table->setItem(0, col, new QTableWidgetItem("—"));
        }
        table->setSpan(0, 0, 1, 8);
        QTableWidgetItem* placeholder = table->item(0, 0);
        placeholder->setText("No logs to display.");
        placeholder->setTextAlignment(Qt::AlignCenter);
        QFont italicFont;
        italicFont.setItalic(true);
        placeholder->setFont(italicFont);
        placeholder->setForeground(QBrush(QColor(120, 120, 120)));
    }

    pageLabel->setText(QString("Page %1 of %2").arg(currentPage + 1).arg(totalPages));
    prevBtn->setEnabled(currentPage > 0);
    nextBtn->setEnabled(currentPage + 1 < totalPages);

    table->resizeColumnsToContents();
    table->resizeRowsToContents();
}

void LogViewer::onClearLogs() {
    if (QMessageBox::question(this, "Clear Logs", "Are you sure you want to clear all logs?") == QMessageBox::Yes) {
        Logger::instance().clearLogs();
        currentPage = 0;
        refreshLogs();
        QMessageBox::information(this, "Logs Cleared", "All logs have been cleared.");
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