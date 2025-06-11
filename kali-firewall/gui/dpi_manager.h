#pragma once

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include "dpi_engine.h"

class DPImanager : public QWidget {
    Q_OBJECT
public:
    explicit DPImanager(QWidget* parent = nullptr);

private slots:
    void refreshSignatureList();
    void onAddSignature();
    void onRemoveSignature();
    void onTestDPI();

private:
    DPIEngine* dpiEngine;

    // Signature management widgets
    QListWidget* sigList;
    QLineEdit* sigNameEdit;
    QLineEdit* sigRegexEdit;
    QComboBox* sigResultBox;
    QComboBox* caseInsensitiveBox;
    QPushButton* addBtn;
    QPushButton* removeBtn;

    // DPI test widgets
    QTextEdit* testPayloadEdit;
    QPushButton* testBtn;
    QLabel* testResultLabel;
};