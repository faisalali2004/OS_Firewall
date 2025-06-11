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
#include <QRegularExpression>
#include <QGroupBox>
#include <QHeaderView>

DPImanager::DPImanager(QWidget* parent)
    : QWidget(parent),
      dpiEngine(new DPIEngine())
{
    auto* mainLayout = new QVBoxLayout(this);

    // --- Signature List ---
    auto* sigLabel = new QLabel("Current DPI Signatures:", this);
    sigList = new QListWidget(this);
    sigList->setSelectionMode(QAbstractItemView::SingleSelection);
    refreshSignatureList();

    // --- Add/Remove Signature Controls ---
    auto* addLayout = new QFormLayout;
    sigNameEdit = new QLineEdit(this);
    sigRegexEdit = new QLineEdit(this);
    sigResultBox = new QComboBox(this);
    sigResultBox->addItems({"Allow", "Block", "HTTP", "DNS", "TLS", "SSH", "FTP", "SMTP", "QUIC", "NONE", "UNKNOWN"});
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
    for (const auto& info : dpiEngine->listSignatures()) {
        QString display = QString("%1 [%2] %3")
            .arg(QString::fromStdString(info.name))
            .arg(info.case_insensitive ? "i" : "")
            .arg(QString::fromStdString(info.regex_str));
        sigList->addItem(display);
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
    if      (resultStr == "Allow")  result = DPIResult::Allow;
    else if (resultStr == "Block")  result = DPIResult::Block;
    else if (resultStr == "HTTP")   result = DPIResult::HTTP;
    else if (resultStr == "DNS")    result = DPIResult::DNS;
    else if (resultStr == "TLS")    result = DPIResult::TLS;
    else if (resultStr == "SSH")    result = DPIResult::SSH;
    else if (resultStr == "FTP")    result = DPIResult::FTP;
    else if (resultStr == "SMTP")   result = DPIResult::SMTP;
    else if (resultStr == "QUIC")   result = DPIResult::QUIC;
    else if (resultStr == "NONE")   result = DPIResult::NONE;
    else if (resultStr == "UNKNOWN")result = DPIResult::UNKNOWN;

    if (dpiEngine->addSignature(name.toStdString(), regex.toStdString(), result, caseInsensitive)) {
        refreshSignatureList();
        QMessageBox::information(this, "Signature Added", "Signature added successfully.");
        sigNameEdit->clear();
        sigRegexEdit->clear();
        sigResultBox->setCurrentIndex(0);
        caseInsensitiveBox->setCurrentIndex(0);
    } else {
        QMessageBox::warning(this, "Duplicate Signature", "A signature with this name already exists.");
    }
}

void DPImanager::onRemoveSignature() {
    auto* item = sigList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Remove Signature", "Select a signature to remove.");
        return;
    }
    // Extract name from display string
    QString display = item->text();
    QString name = display.section(' ', 0, 0);
    if (dpiEngine->removeSignature(name.toStdString())) {
        refreshSignatureList();
        QMessageBox::information(this, "Signature Removed", "Signature removed successfully.");
    } else {
        QMessageBox::warning(this, "Remove Signature", "Signature not found.");
    }
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
        case DPIResult::Allow:    resStr = "Allow"; break;
        case DPIResult::Block:    resStr = "Block"; break;
        case DPIResult::HTTP:     resStr = "HTTP"; break;
        case DPIResult::DNS:      resStr = "DNS"; break;
        case DPIResult::TLS:      resStr = "TLS"; break;
        case DPIResult::SSH:      resStr = "SSH"; break;
        case DPIResult::FTP:      resStr = "FTP"; break;
        case DPIResult::SMTP:     resStr = "SMTP"; break;
        case DPIResult::QUIC:     resStr = "QUIC"; break;
        case DPIResult::NONE:     resStr = "NONE"; break;
        case DPIResult::UNKNOWN:  resStr = "UNKNOWN"; break;
        default:                  resStr = "UNKNOWN"; break;
    }
    QString matchStr = matched.empty() ? "(none)" : QString::fromStdString(matched);
    testResultLabel->setText("Result: " + resStr + "\nMatched: " + matchStr);
}