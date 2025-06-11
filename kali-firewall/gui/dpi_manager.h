#pragma once

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include "core/dpi_engine.h" // Use the core DPI engine and enums

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

    QListWidget* sigList;
    QLineEdit* sigNameEdit;
    QLineEdit* sigRegexEdit;
    QComboBox* sigResultBox;
    QComboBox* caseInsensitiveBox;
    QPushButton* addBtn;
    QPushButton* removeBtn;

    QTextEdit* testPayloadEdit;
    QPushButton* testBtn;
    QLabel* testResultLabel;
};