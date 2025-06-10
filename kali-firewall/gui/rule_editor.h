#include "traffic_shaper_ui.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QNetworkInterface>

TrafficShaperUI::TrafficShaperUI(QWidget* parent)
    : QWidget(parent),
      ifaceCombo(new QComboBox(this)),
      rateEdit(new QLineEdit("1024", this)),
      applyBtn(new QPushButton("Apply Shaping", this)),
      clearBtn(new QPushButton("Clear Shaping", this)),
      statusLabel(new QLabel(this))
{
    populateInterfaces();
    rateEdit->setValidator(new QIntValidator(1, 1000000, this));
    rateEdit->setToolTip("Enter rate in kbit/s (1 - 1,000,000)");

    auto* ifaceLayout = new QHBoxLayout;
    ifaceLayout->addWidget(new QLabel("Interface:"));
    ifaceLayout->addWidget(ifaceCombo);

    auto* rateLayout = new QHBoxLayout;
    rateLayout->addWidget(new QLabel("Rate (kbit/s):"));
    rateLayout->addWidget(rateEdit);

    auto* btnLayout = new QHBoxLayout;
    btnLayout->addWidget(applyBtn);
    btnLayout->addWidget(clearBtn);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(ifaceLayout);
    mainLayout->addLayout(rateLayout);
    mainLayout->addLayout(btnLayout);
    mainLayout->addWidget(statusLabel);

    setLayout(mainLayout);

    connect(applyBtn, &QPushButton::clicked, this, &TrafficShaperUI::applyShaping);
    connect(clearBtn, &QPushButton::clicked, this, &TrafficShaperUI::clearShaping);

    updateStatus("Ready.");
}

TrafficShaperUI::~TrafficShaperUI() = default;

void TrafficShaperUI::populateInterfaces() {
    ifaceCombo->clear();
    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const auto& iface : interfaces) {
        if (iface.flags().testFlag(QNetworkInterface::IsUp) &&
            !iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            ifaceCombo->addItem(iface.humanReadableName());
        }
    }
    if (ifaceCombo->count() == 0)
        ifaceCombo->addItem("eth0"); // fallback
}

bool TrafficShaperUI::validateInput(QString& iface, int& rate) {
    iface = ifaceCombo->currentText().trimmed();
    bool ok = false;
    rate = rateEdit->text().toInt(&ok);
    if (iface.isEmpty()) {
        updateStatus("Error: Interface cannot be empty.", true);
        return false;
    }
    if (!ok || rate <= 0) {
        updateStatus("Error: Invalid rate value.", true);
        return false;
    }
    return true;
}

void TrafficShaperUI::applyShaping() {
    QString iface;
    int rate;
    if (!validateInput(iface, rate))
        return;

    // Emit signal for backend logic
    emit shapingApplied(iface, rate);

    updateStatus(QString("Applied shaping: %1 at %2 kbit/s").arg(iface).arg(rate));
}

void TrafficShaperUI::clearShaping() {
    QString iface = ifaceCombo->currentText().trimmed();
    if (iface.isEmpty()) {
        updateStatus("Error: Interface cannot be empty.", true);
        return;
    }

    // Emit signal for backend logic
    emit shapingCleared(iface);

    updateStatus(QString("Cleared shaping on %1").arg(iface));
}

void TrafficShaperUI::updateStatus(const QString& msg, bool error) {
    statusLabel->setText(msg);
    QPalette pal = statusLabel->palette();
    pal.setColor(QPalette::WindowText, error ? Qt::red : Qt::darkGreen);
    statusLabel->setPalette(pal);
}