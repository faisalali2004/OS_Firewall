#include "traffic_shaper_ui.h"
#include "../core/traffic_shaper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>    

TrafficShaperUI::TrafficShaperUI(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);

    auto* ifaceLayout = new QHBoxLayout();
    ifaceLayout->addWidget(new QLabel("Interface:"));
    ifaceEdit = new QLineEdit("eth0", this);
    ifaceLayout->addWidget(ifaceEdit);

    auto* rateLayout = new QHBoxLayout();
    rateLayout->addWidget(new QLabel("Rate (kbit/s):"));
    rateEdit = new QLineEdit("1024", this);
    rateLayout->addWidget(rateEdit);

    applyBtn = new QPushButton("Apply Shaping", this);
    clearBtn = new QPushButton("Clear Shaping", this);
    statusLabel = new QLabel(this);

    layout->addLayout(ifaceLayout);
    layout->addLayout(rateLayout);
    layout->addWidget(applyBtn);
    layout->addWidget(clearBtn);
    layout->addWidget(statusLabel);

    connect(applyBtn, &QPushButton::clicked, this, &TrafficShaperUI::applyShaping);
    connect(clearBtn, &QPushButton::clicked, this, &TrafficShaperUI::clearShaping);
}

void TrafficShaperUI::applyShaping() {
    QString iface = ifaceEdit->text();
    int rate = rateEdit->text().toInt();
    TrafficShaper shaper;
    if (shaper.shape(iface.toStdString(), rate)) {
        statusLabel->setText("Shaping applied.");
    } else {
        statusLabel->setText("Failed to apply shaping.");
    }
}

void TrafficShaperUI::clearShaping() {
    QString iface = ifaceEdit->text();
    TrafficShaper shaper;
    if (shaper.clear(iface.toStdString())) {
        statusLabel->setText("Shaping cleared.");
    } else {
        statusLabel->setText("Failed to clear shaping.");
    }
}