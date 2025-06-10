#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QTimer>
#include <QPushButton>
#include <QLabel>

class LogViewer : public QWidget {
    Q_OBJECT

public:
    explicit LogViewer(QWidget* parent = nullptr);
    ~LogViewer();

private slots:
    void refreshLogs();
    void manualRefresh();

private:
    void loadLogs();
    void updateStatus(const QString& msg, bool error = false);

    QTableWidget* table;
    QTimer* timer;
    QPushButton* refreshBtn;
    QLabel* statusLabel;
    QString logPath;
};