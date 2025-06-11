#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QToolBar>
#include <QAction>
#include <QToolButton>
#include "dashboard.h"
#include "log_viewer.h"
#include "rule_editor.h"
#include "traffic_shaper_ui.h"
#include "dpi_manager.h"
#include "rule_engine.h"

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
};