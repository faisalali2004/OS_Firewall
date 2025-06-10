#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QTimer>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

class Dashboard : public QWidget {
    Q_OBJECT

public:
    explicit Dashboard(QWidget* parent = nullptr);
    ~Dashboard();

signals:
    void openLogViewer();
    void openRuleEditor();
    void openTrafficShaper();
    void openDPIManager();

private slots:
    void updateStats();

private:
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

    void setupUI();
};