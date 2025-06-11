#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QToolBar>
#include <QAction>
#include <QToolButton>

// Forward declarations to avoid circular includes
class Dashboard;
class LogViewer;
class RuleEditor;
class TrafficShaperUI;
class DPImanager;
class RuleEngine;
class PacketCapture;
class DPIEngine;
struct PacketInfo;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void showDashboard();
    void showLogViewer();
    void showRuleEditor();
    void showTrafficShaper();
    void showDPIManager();

    void onUserDecisionNeeded(const PacketInfo& pkt);
    void onInteractiveModeToggled(bool checked);

    void updateStatsDisplay(int total, int blocked, int memoryKB);

private:
    void setupNavigation();
    void setupConnections();

    QStackedWidget* stackedWidget;
    Dashboard* dashboard;
    LogViewer* logViewer;
    RuleEditor* ruleEditor;
    TrafficShaperUI* trafficShaperUI;
    DPImanager* dpiManager;

    QToolBar* navToolBar;
    QAction* dashboardAction;
    QAction* logAction;
    QAction* ruleAction;
    QAction* shaperAction;
    QAction* dpiAction;
    QToolButton* interactiveModeButton;

    RuleEngine* ruleEngine;
    PacketCapture* packetCapture;
    DPIEngine* dpiEngine;
};