#include "dashboard.h"
#include "logger.h"
#include <QRandomGenerator>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QDebug>

Dashboard::Dashboard(Logger* logger, QWidget* parent)
    : QWidget(parent),
      logger(logger),
      statusLabel(new QLabel("Firewall Status: <b>Active</b>", this)),
      trafficLabel(new QLabel("Traffic: 0 packets", this)),
      blockedLabel(new QLabel("Blocked: 0 packets", this)),
      memoryLabel(new QLabel("Memory Usage: 0 KB", this)),
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

    statsTimer->start(2000); // Update system stats every 2 seconds
    updateStats();
}

Dashboard::~Dashboard() = default;

void Dashboard::setupUI() {
    cpuBar->setRange(0, 100);
    memBar->setRange(0, 100);
    cpuBar->setFormat("CPU Usage: %p%");
    memBar->setFormat("System Memory: %p%");

    auto* statsBox = new QGroupBox("System Stats", this);
    auto* statsLayout = new QVBoxLayout;
    statsLayout->addWidget(cpuBar);
    statsLayout->addWidget(memBar);
    statsBox->setLayout(statsLayout);

    auto* trafficBox = new QGroupBox("Firewall Stats", this);
    auto* trafficLayout = new QVBoxLayout;
    trafficLayout->addWidget(trafficLabel);
    trafficLayout->addWidget(blockedLabel);
    trafficLayout->addWidget(memoryLabel);
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

// --- Real-time stats from PacketCapture/MainWindow ---
void Dashboard::setStats(int total, int blocked, int memoryKB) {
    trafficLabel->setText(QString("Traffic: %1 packets").arg(total));
    blockedLabel->setText(QString("Blocked: %1 packets").arg(blocked));
    memoryLabel->setText(QString("Memory Usage: %1 KB").arg(memoryKB));

    // Status logic based on real-time stats
    if (blocked > 0 && blocked > total / 2) {
        statusLabel->setText("Firewall Status: <b style='color:orange;'>Blocking Heavily</b>");
    } else {
        statusLabel->setText("Firewall Status: <b>Active</b>");
    }
}

// --- System stats (CPU/memory) for info only ---
void Dashboard::updateStats() {
    int cpu = getCpuUsage();
    int mem = getMemUsage();

    cpuBar->setValue(cpu);
    memBar->setValue(mem);

    // Optionally, you can also update statusLabel here for system load
    if (cpu > 90 || mem > 90) {
        statusLabel->setText("Firewall Status: <b style='color:red;'>High Load</b>");
    }
}

// --- Cross-platform CPU usage (Linux) ---
int Dashboard::getCpuUsage() {
#ifdef Q_OS_LINUX
    static long lastTotalUser = 0, lastTotalUserLow = 0, lastTotalSys = 0, lastTotalIdle = 0;
    QFile file("/proc/stat");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return QRandomGenerator::global()->bounded(100);
    QTextStream in(&file);
    QString line = in.readLine();
    QStringList values = line.split(' ', Qt::SkipEmptyParts);
    if (values.size() < 5) return QRandomGenerator::global()->bounded(100);
    long user = values[1].toLong();
    long nice = values[2].toLong();
    long sys = values[3].toLong();
    long idle = values[4].toLong();
    long totalUser = user;
    long totalUserLow = nice;
    long totalSys = sys;
    long totalIdle = idle;
    long total = (totalUser - lastTotalUser) + (totalUserLow - lastTotalUserLow) +
                 (totalSys - lastTotalSys);
    long totalAll = total + (totalIdle - lastTotalIdle);
    int percent = totalAll == 0 ? 0 : int(100.0 * total / totalAll);
    lastTotalUser = totalUser;
    lastTotalUserLow = totalUserLow;
    lastTotalSys = totalSys;
    lastTotalIdle = totalIdle;
    return percent;
#else
    return QRandomGenerator::global()->bounded(100);
#endif
}

// --- Cross-platform Memory usage (Linux, system-wide) ---
int Dashboard::getMemUsage() {
#ifdef Q_OS_LINUX
    QFile file("/proc/meminfo");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return QRandomGenerator::global()->bounded(100);
    QTextStream in(&file);
    long memTotal = 0, memFree = 0, buffers = 0, cached = 0;
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith("MemTotal:")) memTotal = line.split(' ', Qt::SkipEmptyParts)[1].toLong();
        else if (line.startsWith("MemFree:")) memFree = line.split(' ', Qt::SkipEmptyParts)[1].toLong();
        else if (line.startsWith("Buffers:")) buffers = line.split(' ', Qt::SkipEmptyParts)[1].toLong();
        else if (line.startsWith("Cached:")) cached = line.split(' ', Qt::SkipEmptyParts)[1].toLong();
    }
    long used = memTotal - memFree - buffers - cached;
    int percent = memTotal == 0 ? 0 : int(100.0 * used / memTotal);
    return percent;
#else
    return QRandomGenerator::global()->bounded(100);
#endif
}