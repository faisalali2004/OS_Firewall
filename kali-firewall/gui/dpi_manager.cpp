#include "dpi_manager.h"
#include "dpi_engine.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QByteArray>
#include <QTextEdit>

DPImanager::DPImanager(QWidget* parent)
    : QWidget(parent),
      dpiEngine(new DPIEngine())
{
    auto* mainLayout = new QVBoxLayout(this);

    // --- Signature List ---
    auto* sigLabel = new QLabel("Current DPI Signatures:", this);
    sigList = new QListWidget(this);
    refreshSignatureList();

    // --- Add/Remove Signature Controls ---
    auto* addLayout = new QFormLayout;
    sigNameEdit = new QLineEdit(this);
    sigRegexEdit = new QLineEdit(this);
    sigResultBox = new QComboBox(this);
    sigResultBox->addItems({"HTTP", "DNS", "TLS", "SSH", "FTP", "SMTP", "QUIC", "UNKNOWN"});
    caseInsensitiveBox = new QComboBox(this);
    caseInsensitiveBox->addItems({"No", "Yes"});
    addBtn = new QPushButton("Add Signature", this);
    removeBtn = new QPushButton("Remove Selected", this);

    addLayout->addRow("Name:", sigNameEdit);
    addLayout->addRow("Regex:", sigRegexEdit);
    addLayout->addRow("Result:", sigResultBox);
    addLayout->addRow("Case Insensitive:", caseInsensitiveBox);

    auto* btnLayout = new QHBoxLayout;
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(removeBtn);

    // --- Test DPI Section ---
    auto* testLayout = new QFormLayout;
    testPayloadEdit = new QTextEdit(this);
    testPayloadEdit->setPlaceholderText("Enter hex or ASCII payload to test...");
    testBtn = new QPushButton("Test DPI", this);
    testResultLabel = new QLabel(this);

    testLayout->addRow("Test Payload:", testPayloadEdit);
    testLayout->addRow(testBtn);
    testLayout->addRow("Result:", testResultLabel);

    // --- Assemble Layout ---
    mainLayout->addWidget(sigLabel);
    mainLayout->addWidget(sigList);
    mainLayout->addLayout(addLayout);
    mainLayout->addLayout(btnLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(testLayout);

    // --- Connections ---
    connect(addBtn, &QPushButton::clicked, this, &DPImanager::onAddSignature);
    connect(removeBtn, &QPushButton::clicked, this, &DPImanager::onRemoveSignature);
    connect(testBtn, &QPushButton::clicked, this, &DPImanager::onTestDPI);

    setLayout(mainLayout);
}

void DPImanager::refreshSignatureList() {
    sigList->clear();
    for (const auto& name : dpiEngine->listSignatures()) {
        sigList->addItem(QString::fromStdString(name));
    }
}

void DPImanager::onAddSignature() {
    QString name = sigNameEdit->text().trimmed();
    QString regex = sigRegexEdit->text().trimmed();
    QString resultStr = sigResultBox->currentText();
    bool caseInsensitive = (caseInsensitiveBox->currentIndex() == 1);

    if (name.isEmpty() || regex.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Name and Regex must not be empty.");
        return;
    }

    DPIResult result = DPIResult::UNKNOWN;
    if (resultStr == "HTTP") result = DPIResult::HTTP;
    else if (resultStr == "DNS") result = DPIResult::DNS;
    else if (resultStr == "TLS") result = DPIResult::TLS;
    else if (resultStr == "SSH") result = DPIResult::SSH;
    else if (resultStr == "FTP") result = DPIResult::FTP;
    else if (resultStr == "SMTP") result = DPIResult::SMTP;
    else if (resultStr == "QUIC") result = DPIResult::QUIC;

    dpiEngine->addSignature(name.toStdString(), regex.toStdString(), result, caseInsensitive);
    refreshSignatureList();
    QMessageBox::information(this, "Signature Added", "Signature added successfully.");
}

void DPImanager::onRemoveSignature() {
    auto* item = sigList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Remove Signature", "Select a signature to remove.");
        return;
    }
    QString name = item->text();
    dpiEngine->removeSignature(name.toStdString());
    refreshSignatureList();
    QMessageBox::information(this, "Signature Removed", "Signature removed successfully.");
}

void DPImanager::onTestDPI() {
    QString input = testPayloadEdit->toPlainText().trimmed();
    if (input.isEmpty()) {
        testResultLabel->setText("Enter a payload to test.");
        return;
    }

    // Try to interpret as hex, else as ASCII
    QByteArray payload;
    bool isHex = input.contains(QRegularExpression("^[0-9a-fA-F\\s]+$"));
    if (isHex) {
        payload = QByteArray::fromHex(input.remove(' ').toLatin1());
    } else {
        payload = input.toUtf8();
    }

    std::string matched;
    DPIResult res = dpiEngine->inspect(reinterpret_cast<const uint8_t*>(payload.constData()), payload.size(), matched);

    QString resStr;
    switch (res) {
        case DPIResult::HTTP: resStr = "HTTP"; break;
        case DPIResult::DNS: resStr = "DNS"; break;
        case DPIResult::TLS: resStr = "TLS"; break;
        case DPIResult::SSH: resStr = "SSH"; break;
        case DPIResult::FTP: resStr = "FTP"; break;
        case DPIResult::SMTP: resStr = "SMTP"; break;
        case DPIResult::QUIC: resStr = "QUIC"; break;
        case DPIResult::NONE: resStr = "NONE"; break;
        default: resStr = "UNKNOWN"; break;
    }
    testResultLabel->setText("Result: " + resStr + "\nMatched: " + QString::fromStdString(matched));
}