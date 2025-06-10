#pragma once
#include "logger.h"
#include <QWidget>

class Logger;
class QTableWidget;
class QLineEdit;
class QPushButton;
class QLabel;

class LogViewer : public QWidget {
    Q_OBJECT
public:
    explicit LogViewer(Logger* logger, QWidget* parent = nullptr);

private slots:
    void refreshLogs();
    void onClearLogs();
    void onPrevPage();
    void onNextPage();

private:
    Logger* logger;
    QTableWidget* table;
    QLineEdit* searchEdit;
    QPushButton* refreshBtn;
    QPushButton* clearBtn;
    QPushButton* prevBtn;
    QPushButton* nextBtn;
    QLabel* pageLabel;
    int currentPage;
    int pageSize;
};