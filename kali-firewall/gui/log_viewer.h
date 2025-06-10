#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QTimer>

class LogViewer : public QWidget {
    Q_OBJECT
public:
    explicit LogViewer(QWidget* parent = nullptr);
    void refreshLogs();

private:
    QTableWidget* table;
    QTimer* timer;
    void loadLogs();
};