#include "mainwindow.h"
#include "dashboard.h"
#include "log_viewer.h"
#include "rule_editor.h"
#include "traffic_shaper_ui.h"
#include "dpi_manager.h"
#include "rule_engine.h"
#include <QMainWindow>
#include <QStackedWidget>
#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QApplication>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      stackedWidget(new QStackedWidget(this)),
      dashboard(new Dashboard(this)),
      logViewer(new LogViewer(this)),
      ruleEditor(new RuleEditor(this)),
      trafficShaperUI(new TrafficShaperUI(this)),
      dpiManager(new DPIManager(this)),
      navToolBar(new QToolBar("Navigation", this)),
      dashboardAction(new QAction(QIcon::fromTheme("view-dashboard"), "Dashboard", this)),
      logAction(new QAction(QIcon::fromTheme("document-open"), "Logs", this)),
      ruleAction(new QAction(QIcon::fromTheme("edit"), "Rules", this)),
      shaperAction(new QAction(QIcon::fromTheme("network-wired"), "Traffic Shaper", this)),
      dpiAction(new QAction(QIcon::fromTheme("security-high"), "DPI Manager", this))
{
    setWindowTitle("Kali Firewall");
    setMinimumSize(900, 600);

    // Add widgets to stackedWidget
    stackedWidget->addWidget(dashboard);      // index 0
    stackedWidget->addWidget(logViewer);      // index 1
    stackedWidget->addWidget(ruleEditor);     // index 2
    stackedWidget->addWidget(trafficShaperUI);// index 3
    stackedWidget->addWidget(dpiManager);     // index 4

    setCentralWidget(stackedWidget);

    setupNavigation();
    setupConnections();

    // --- INTERACTIVE FIREWALL POPUP INTEGRATION ---
    // Make sure ruleEngine is initialized before this!
    // Example: ruleEngine = new RuleEngine(this);
    if (ruleEngine) {
        connect(ruleEngine, &RuleEngine::userDecisionNeeded,
                this, &MainWindow::onUserDecisionNeeded);
    }

    // Show dashboard by default
    showDashboard();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupNavigation() {
    navToolBar->addAction(dashboardAction);
    navToolBar->addAction(logAction);
    navToolBar->addAction(ruleAction);
    navToolBar->addAction(shaperAction);
    navToolBar->addAction(dpiAction);
    addToolBar(Qt::LeftToolBarArea, navToolBar);
    navToolBar->setMovable(false);
}

void MainWindow::setupConnections() {
    connect(dashboardAction, &QAction::triggered, this, &MainWindow::showDashboard);
    connect(logAction, &QAction::triggered, this, &MainWindow::showLogViewer);
    connect(ruleAction, &QAction::triggered, this, &MainWindow::showRuleEditor);
    connect(shaperAction, &QAction::triggered, this, &MainWindow::showTrafficShaper);
    connect(dpiAction, &QAction::triggered, this, &MainWindow::showDPIManager);

    // Dashboard quick access signals
    connect(dashboard, &Dashboard::openLogViewer, this, &MainWindow::showLogViewer);
    connect(dashboard, &Dashboard::openRuleEditor, this, &MainWindow::showRuleEditor);
    connect(dashboard, &Dashboard::openTrafficShaper, this, &MainWindow::showTrafficShaper);
    connect(dashboard, &Dashboard::openDPIManager, this, &MainWindow::showDPIManager);
}

void MainWindow::showDashboard() {
    stackedWidget->setCurrentWidget(dashboard);
}

void MainWindow::showLogViewer() {
    stackedWidget->setCurrentWidget(logViewer);
}

void MainWindow::showRuleEditor() {
    stackedWidget->setCurrentWidget(ruleEditor);
}

void MainWindow::showTrafficShaper() {
    stackedWidget->setCurrentWidget(trafficShaperUI);
}

void MainWindow::showDPIManager() {
    stackedWidget->setCurrentWidget(dpiManager);
}

// --- INTERACTIVE FIREWALL POPUP SLOT ---
void MainWindow::onUserDecisionNeeded(const PacketInfo& pkt) {
    QString msg = QString("A new connection was detected:\n\n"
                          "Source IP: %1\n"
                          "Destination IP: %2\n"
                          "Source Port: %3\n"
                          "Destination Port: %4\n\n"
                          "Do you want to ALLOW this connection?")
                          .arg(pkt.srcIp)
                          .arg(pkt.dstIp)
                          .arg(pkt.srcPort)
                          .arg(pkt.dstPort);

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Firewall Decision", msg, QMessageBox::Yes | QMessageBox::No);

    if (ruleEngine) {
        if (reply == QMessageBox::Yes) {
            ruleEngine->userDecisionReceived("allow");
        } else {
            ruleEngine->userDecisionReceived("block");
        }
    }
}