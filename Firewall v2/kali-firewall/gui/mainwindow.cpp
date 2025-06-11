#include "mainwindow.h"
#include "dpi_manager.h"
#include <QMessageBox>
#include <QIcon>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      stackedWidget(new QStackedWidget(this)),
      dashboard(new Dashboard(&Logger::instance(), this)),
      logViewer(new LogViewer(this)),
      ruleEditor(new RuleEditor(this)),
      trafficShaperUI(new TrafficShaperUI(this)),
      dpiManager(new DPIManager(this)),
      navToolBar(new QToolBar("Navigation", this)),
      dashboardAction(new QAction(QIcon::fromTheme("view-dashboard"), "Dashboard", this)),
      logAction(new QAction(QIcon::fromTheme("document-open"), "Logs", this)),
      ruleAction(new QAction(QIcon::fromTheme("edit"), "Rules", this)),
      shaperAction(new QAction(QIcon::fromTheme("network-wired"), "Traffic Shaper", this)),
      dpiAction(new QAction(QIcon::fromTheme("security-high"), "DPI Manager", this)),
      interactiveModeButton(new QToolButton(this)),
      ruleEngine(new RuleEngine(this, "../config/default_rules.json")),
      packetCapture(new PacketCapture),
      dpiEngine(new DPIEngine)
{
    setWindowTitle("Kali Firewall");
    setMinimumSize(900, 600);

    if (!Logger::instance().initDB("../logs/firewall_log.db")) {
        QMessageBox::critical(this, "Logger Error", "Failed to initialize log database. Logging will be disabled.");
    }

    stackedWidget->addWidget(dashboard);
    stackedWidget->addWidget(logViewer);
    stackedWidget->addWidget(ruleEditor);
    stackedWidget->addWidget(trafficShaperUI);
    stackedWidget->addWidget(dpiManager);

    setCentralWidget(stackedWidget);

    setupNavigation();
    setupConnections();

    // Interactive mode
    connect(ruleEngine, &RuleEngine::userDecisionNeeded,
            this, &MainWindow::onUserDecisionNeeded);
    ruleEngine->setInteractiveMode(true);

    interactiveModeButton->setText("Interactive Mode");
    interactiveModeButton->setCheckable(true);
    interactiveModeButton->setChecked(true);
    navToolBar->addWidget(interactiveModeButton);
    connect(interactiveModeButton, &QToolButton::toggled,
            this, &MainWindow::onInteractiveModeToggled);

    // Packet capture integration
    packetCapture->setRuleEngine(ruleEngine);
    packetCapture->setDPIEngine(dpiEngine);

    connect(packetCapture, &PacketCapture::statsUpdated,
            this, &MainWindow::updateStatsDisplay);

    packetCapture->init(0);
    packetCapture->start();

    updateStatsDisplay(0, 0, 0);

    showDashboard();
}

MainWindow::~MainWindow() {
    packetCapture->stop();
    delete packetCapture;
    delete dpiEngine;
}

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

    connect(dashboard, &Dashboard::openLogViewer, this, &MainWindow::showLogViewer);
    connect(dashboard, &Dashboard::openRuleEditor, this, &MainWindow::showRuleEditor);
    connect(dashboard, &Dashboard::openTrafficShaper, this, &MainWindow::showTrafficShaper);
    connect(dashboard, &Dashboard::openDPIManager, this, &MainWindow::showDPIManager);
}

void MainWindow::showDashboard() { stackedWidget->setCurrentWidget(dashboard); }
void MainWindow::showLogViewer() { stackedWidget->setCurrentWidget(logViewer); }
void MainWindow::showRuleEditor() { stackedWidget->setCurrentWidget(ruleEditor); }
void MainWindow::showTrafficShaper() { stackedWidget->setCurrentWidget(trafficShaperUI); }
void MainWindow::showDPIManager() { stackedWidget->setCurrentWidget(dpiManager); }

void MainWindow::onUserDecisionNeeded(const PacketInfo& pkt) {
    QString msg = QString("A new connection was detected:\n\n"
                          "Source IP: %1\n"
                          "Destination IP: %2\n"
                          "Source Port: %3\n"
                          "Destination Port: %4\n"
                          "Protocol: %5\n\n"
                          "Do you want to ALLOW this connection?")
                          .arg(pkt.srcIp)
                          .arg(pkt.dstIp)
                          .arg(pkt.srcPort)
                          .arg(pkt.dstPort)
                          .arg(pkt.protocol);

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

void MainWindow::onInteractiveModeToggled(bool checked) {
    if (ruleEngine) {
        ruleEngine->setInteractiveMode(checked);
        QString msg = checked ? "Interactive mode enabled.\nYou will be prompted for unknown connections."
                              : "Interactive mode disabled.\nUnknown connections will be blocked by default.";
        QMessageBox::information(this, "Interactive Mode", msg);
    }
}

void MainWindow::updateStatsDisplay(int total, int blocked, int memoryKB) {
    dashboard->setStats(total, blocked, memoryKB);
}