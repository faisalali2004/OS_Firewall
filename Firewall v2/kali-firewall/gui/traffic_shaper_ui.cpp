#include "traffic_shaper_ui.h"
#include "traffic_shaper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>
#include <QNetworkInterface>
#include <QMovie>
#include <QTimer>

TrafficShaperUI::TrafficShaperUI(QWidget* parent)
    : QWidget(parent),
      shaper(new TrafficShaper(this))
{
    // --- UI Elements ---
    auto* layout = new QVBoxLayout(this);

    auto* formLayout = new QFormLayout;
    ifaceBox = new QComboBox(this);
    refreshBtn = new QPushButton("⟳", this);
    refreshBtn->setFixedWidth(32);

    auto* ifaceLayout = new QHBoxLayout;
    ifaceLayout->addWidget(ifaceBox);
    ifaceLayout->addWidget(refreshBtn);
    formLayout->addRow("Interface:", ifaceLayout);

    rateSpin = new QSpinBox(this);
    burstSpin = new QSpinBox(this);
    latencySpin = new QSpinBox(this);

    rateSpin->setRange(1, 1000000);      // kbit/s
    rateSpin->setValue(1000);
    burstSpin->setRange(1, 100000);
    burstSpin->setValue(32);
    latencySpin->setRange(0, 10000);     // ms
    latencySpin->setValue(400);

    formLayout->addRow("Rate (kbit/s):", rateSpin);
    formLayout->addRow("Burst (kbit):", burstSpin);
    formLayout->addRow("Latency (ms):", latencySpin);

    layout->addLayout(formLayout);

    auto* btnLayout = new QHBoxLayout;
    applyBtn = new QPushButton("Apply Shaping", this);
    clearBtn = new QPushButton("Clear Shaping", this);
    btnLayout->addWidget(applyBtn);
    btnLayout->addWidget(clearBtn);
    layout->addLayout(btnLayout);

    statusLabel = new QLabel(this);
    statusLabel->setText("Ready.");
    layout->addWidget(statusLabel);

    busyLabel = new QLabel(this);
    busyLabel->setVisible(false);
    layout->addWidget(busyLabel);

    // --- Connections ---
    connect(applyBtn, &QPushButton::clicked, this, &TrafficShaperUI::onApplyClicked);
    connect(clearBtn, &QPushButton::clicked, this, &TrafficShaperUI::onClearClicked);
    connect(refreshBtn, &QPushButton::clicked, this, &TrafficShaperUI::refreshInterfaces);

    connect(shaper, &TrafficShaper::shapingSuccess, this, [=](const QString& iface){
        setStatus("Shaping applied to " + iface, Qt::darkGreen);
        setBusy(false);
        QMessageBox::information(this, "Traffic Shaper", "Shaping applied to " + iface);
    });
    connect(shaper, &TrafficShaper::shapingError, this, [=](const QString& iface, const QString& err){
        setStatus("Error: " + err, Qt::darkRed);
        setBusy(false);
        QMessageBox::critical(this, "Traffic Shaper Error", iface + ":\n" + err);
    });
    connect(shaper, &TrafficShaper::cleared, this, [=](const QString& iface){
        setStatus("Shaping cleared on " + iface, Qt::darkGreen);
        setBusy(false);
        QMessageBox::information(this, "Traffic Shaper", "Shaping cleared on " + iface);
    });
    connect(shaper, &TrafficShaper::clearError, this, [=](const QString& iface, const QString& err){
        setStatus("Error: " + err, Qt::darkRed);
        setBusy(false);
        QMessageBox::warning(this, "Traffic Shaper", iface + ":\n" + err);
    });

    refreshInterfaces();
}

void TrafficShaperUI::refreshInterfaces() {
    ifaceBox->clear();
    for (const QNetworkInterface& iface : QNetworkInterface::allInterfaces()) {
        if (iface.flags().testFlag(QNetworkInterface::IsUp) &&
            iface.flags().testFlag(QNetworkInterface::IsRunning) &&
            !iface.humanReadableName().startsWith("lo")) {
            ifaceBox->addItem(iface.humanReadableName());
        }
    }
    if (ifaceBox->count() == 0) {
        setStatus("No active interfaces found.", Qt::darkRed);
    } else {
        setStatus("Ready.", Qt::blue);
    }
}

void TrafficShaperUI::onApplyClicked() {
    QString iface = ifaceBox->currentText();
    int rate = rateSpin->value();
    int burst = burstSpin->value();
    int latency = latencySpin->value();

    // --- Input validation ---
    if (iface.isEmpty()) {
        setStatus("Please select a network interface.", Qt::darkRed);
        QMessageBox::warning(this, "Input Error", "Please select a network interface.");
        return;
    }
    if (rate <= 0 || burst <= 0 || latency < 0) {
        setStatus("Invalid parameters.", Qt::darkRed);
        QMessageBox::warning(this, "Input Error", "Rate, burst, and latency must be positive.");
        return;
    }

    setBusy(true);
    setStatus("Applying shaping...", Qt::blue);
    shaper->shape(iface, rate, burst, latency);
}

void TrafficShaperUI::onClearClicked() {
    QString iface = ifaceBox->currentText();
    if (iface.isEmpty()) {
        setStatus("Please select a network interface.", Qt::darkRed);
        QMessageBox::warning(this, "Input Error", "Please select a network interface.");
        return;
    }
    setBusy(true);
    setStatus("Clearing shaping...", Qt::blue);
    shaper->clear(iface);
}

void TrafficShaperUI::setStatus(const QString& text, QColor color) {
    statusLabel->setText(text);
    QPalette pal = statusLabel->palette();
    pal.setColor(QPalette::WindowText, color);
    statusLabel->setPalette(pal);
}

void TrafficShaperUI::setBusy(bool busy) {
    if (busy) {
        busyLabel->setVisible(true);
        busyLabel->setText("Working...");
        // Optionally, use a spinner GIF:
        // QMovie* movie = new QMovie(":/icons/spinner.gif", QByteArray(), busyLabel);
        // busyLabel->setMovie(movie);
        // movie->start();
    } else {
        busyLabel->setVisible(false);
        busyLabel->clear();
    }
}