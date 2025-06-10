#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QToolBar>
#include <QAction>
#include <QToolButton>
#include "rule_engine.h" // For PacketInfo and RuleEngine

class Dashboard;
class LogViewer;
class RuleEditor;
class TrafficShaperUI;
class DPImanager; // Note: class name matches your implementation
class Logger;

/**
 * @brief MainWindow is the central widget for the firewall GUI.
 *        It manages navigation, logger integration, and interactive firewall popups.
 */
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

    // Interactive firewall popup slot
    void onUserDecisionNeeded(const PacketInfo& pkt);

    // Slot for interactive mode toggle
    void onInteractiveModeToggled(bool checked);

private:
    void setupNavigation();
    void setupConnections();

    Logger* logger;
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

    // Interactive mode toggle button
    QToolButton* interactiveModeButton;

    // Pointer to your RuleEngine instance
    RuleEngine* ruleEngine;
};