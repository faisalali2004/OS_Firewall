#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <vector>
#include "logger.h" // This should provide the standalone LogEntry struct

class LogViewer : public QWidget {
    Q_OBJECT
public:
    explicit LogViewer(QWidget* parent = nullptr);

private slots:
    void refreshLogs();
    void onClearLogs();
    void onPrevPage();
    void onNextPage();

private:
    QTableWidget* table;
    QPushButton* prevBtn;
    QPushButton* nextBtn;
    QPushButton* clearBtn;
    QLabel* pageLabel;

    int currentPage;
    int pageSize;
    std::vector<LogEntry> logs; // Uses the standalone LogEntry struct
};