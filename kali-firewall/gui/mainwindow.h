#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QToolBar>
#include <QAction>

class Dashboard;
class LogViewer;
class RuleEditor;
class TrafficShaperUI;
class DPIManager;

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

private:
    QStackedWidget* stackedWidget;
    Dashboard* dashboard;
    LogViewer* logViewer;
    RuleEditor* ruleEditor;
    TrafficShaperUI* trafficShaperUI;
    DPIManager* dpiManager;

    QToolBar* navToolBar;
    QAction* dashboardAction;
    QAction* logAction;
    QAction* ruleAction;
    QAction* shaperAction;
    QAction* dpiAction;

    void setupNavigation();
    void setupConnections();
};