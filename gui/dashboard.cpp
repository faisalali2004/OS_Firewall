#include "dashboard.h"
#include <QApplication>
#include <QDateTime>
#include <QRandomGenerator>

Dashboard::Dashboard(QWidget* parent)
    : QWidget(parent),
      statusLabel(new QLabel("Firewall Status: <b>Active</b>", this)),
      trafficLabel(new QLabel("Traffic: 0 packets", this)),
      blockedLabel(new QLabel("Blocked: 0 packets", this)),
      cpuBar(new QProgressBar(this)),
      memBar(new QProgressBar(this)),
      statsTimer(new QTimer(this)),
      logBtn(new QPushButton("View Logs", this)),
      ruleBtn(new QPushButton("Edit Rules", this)),
      shaperBtn(new QPushButton("Traffic Shaper", this)),
      dpiBtn(new QPushButton("DPI Manager", this))
{
    setupUI();

    connect(statsTimer, &QTimer::timeout, this, &Dashboard::updateStats);
    connect(logBtn, &QPushButton::clicked, this, &Dashboard::openLogViewer);
    connect(ruleBtn, &QPushButton::clicked, this, &Dashboard::openRuleEditor);
    connect(shaperBtn, &QPushButton::clicked, this, &Dashboard::openTrafficShaper);
    connect(dpiBtn, &QPushButton::clicked, this, &Dashboard::openDPIManager);

    statsTimer->start(2000); // Update stats every 2 seconds
    updateStats();
}

Dashboard::~Dashboard() = default;

void Dashboard::setupUI() {
    cpuBar->setRange(0, 100);
    memBar->setRange(0, 100);
    cpuBar->setFormat("CPU Usage: %p%");
    memBar->setFormat("Memory Usage: %p%");

    auto* statsBox = new QGroupBox("System Stats", this);
    auto* statsLayout = new QVBoxLayout;
    statsLayout->addWidget(cpuBar);
    statsLayout->addWidget(memBar);
    statsBox->setLayout(statsLayout);

    auto* trafficBox = new QGroupBox("Traffic Overview", this);
    auto* trafficLayout = new QVBoxLayout;
    trafficLayout->addWidget(trafficLabel);
    trafficLayout->addWidget(blockedLabel);
    trafficBox->setLayout(trafficLayout);

    auto* btnLayout = new QHBoxLayout;
    btnLayout->addWidget(logBtn);
    btnLayout->addWidget(ruleBtn);
    btnLayout->addWidget(shaperBtn);
    btnLayout->addWidget(dpiBtn);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(statusLabel);
    mainLayout->addWidget(statsBox);
    mainLayout->addWidget(trafficBox);
    mainLayout->addLayout(btnLayout);

    setLayout(mainLayout);
    setMinimumWidth(400);
}

void Dashboard::updateStats() {
    // Simulate stats for demo; replace with real data in integration
    int cpu = QRandomGenerator::global()->bounded(100);
    int mem = QRandomGenerator::global()->bounded(100);
    int traffic = QRandomGenerator::global()->bounded(10000);
    int blocked = QRandomGenerator::global()->bounded(1000);

    cpuBar->setValue(cpu);
    memBar->setValue(mem);
    trafficLabel->setText(QString("Traffic: %1 packets").arg(traffic));
    blockedLabel->setText(QString("Blocked: %1 packets").arg(blocked));

    // Optionally, update status based on conditions
    if (cpu > 90 || mem > 90) {
        statusLabel->setText("Firewall Status: <b style='color:red;'>High Load</b>");
    } else {
        statusLabel->setText("Firewall Status: <b>Active</b>");
    }
}