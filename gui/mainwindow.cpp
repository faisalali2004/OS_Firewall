#include "mainwindow.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Kali Firewall Dashboard");
    resize(900, 600);

    auto* tabs = new QTabWidget(this);

    // Dashboard tab
    auto* dashboard = new QWidget();
    auto* dashLayout = new QVBoxLayout();
    dashLayout->addWidget(new QLabel("Dashboard: Real-time stats, active connections, etc."));
    dashboard->setLayout(dashLayout);
    tabs->addTab(dashboard, "Dashboard");

    // Rules tab
    auto* rules = new QWidget();
    auto* rulesLayout = new QVBoxLayout();
    rulesLayout->addWidget(new QLabel("Rule Editor: Add/Edit/Delete firewall rules."));
    rules->setLayout(rulesLayout);
    tabs->addTab(rules, "Rules");

    // Logs tab
    auto* logs = new QWidget();
    auto* logsLayout = new QVBoxLayout();
    logsLayout->addWidget(new QLabel("Log Viewer: Real-time firewall logs."));
    logs->setLayout(logsLayout);
    tabs->addTab(logs, "Logs");

    // Traffic Shaper tab
    auto* shaper = new QWidget();
    auto* shaperLayout = new QVBoxLayout();
    shaperLayout->addWidget(new QLabel("Traffic Shaper: Bandwidth control."));
    shaper->setLayout(shaperLayout);
    tabs->addTab(shaper, "Traffic Shaper");

    // DPI tab
    auto* dpi = new QWidget();
    auto* dpiLayout = new QVBoxLayout();
    dpiLayout->addWidget(new QLabel("DPI Manager: Protocol inspection plugins."));
    dpi->setLayout(dpiLayout);
    tabs->addTab(dpi, "DPI");

    setCentralWidget(tabs);
}

MainWindow::~MainWindow() {}