#pragma once

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>
#include "logger.h"

class Dashboard : public QWidget {
    Q_OBJECT
public:
    explicit Dashboard(Logger* logger, QWidget* parent = nullptr);
    ~Dashboard();

signals:
    void openLogViewer();
    void openRuleEditor();
    void openTrafficShaper();
    void openDPIManager();

public slots:
    void setStats(int total, int blocked, int memoryKB);

private slots:
    void updateStats();

private:
    void setupUI();

    Logger* logger;
    QLabel* statusLabel;
    QLabel* trafficLabel;
    QLabel* blockedLabel;
    QLabel* memoryLabel;
    QProgressBar* cpuBar;
    QProgressBar* memBar;
    QTimer* statsTimer;
    QPushButton* logBtn;
    QPushButton* ruleBtn;
    QPushButton* shaperBtn;
    QPushButton* dpiBtn;

    int getCpuUsage();
    int getMemUsage();
};