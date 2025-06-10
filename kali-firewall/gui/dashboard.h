#pragma once

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>

class Logger;

/**
 * @brief Dashboard widget showing firewall status, system stats, and navigation.
 */
class Dashboard : public QWidget {
    Q_OBJECT
public:
    explicit Dashboard(Logger* logger = nullptr, QWidget* parent = nullptr);
    ~Dashboard();

signals:
    void openLogViewer();
    void openRuleEditor();
    void openTrafficShaper();
    void openDPIManager();

private slots:
    void updateStats();

private:
    void setupUI();
    int getCpuUsage();
    int getMemUsage();

    Logger* logger;
    QLabel* statusLabel;
    QLabel* trafficLabel;
    QLabel* blockedLabel;
    QProgressBar* cpuBar;
    QProgressBar* memBar;
    QTimer* statsTimer;
    QPushButton* logBtn;
    QPushButton* ruleBtn;
    QPushButton* shaperBtn;
    QPushButton* dpiBtn;
};