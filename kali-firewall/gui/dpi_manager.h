#pragma once
#include <QWidget>

/**
 * @brief DPImanager provides a GUI for managing DPI signatures and testing DPI detection.
 *        Users can add/remove signatures, view all signatures, and test payloads (hex or ASCII).
 *        Integrates with DPIEngine for real-time inspection and management.
 */
class DPIEngine;
class QListWidget;
class QLineEdit;
class QComboBox;
class QPushButton;
class QLabel;
class QTextEdit;

class DPImanager : public QWidget {
    Q_OBJECT
public:
    /**
     * @brief Construct a new DPImanager widget.
     * @param parent Parent widget.
     */
    explicit DPImanager(QWidget* parent = nullptr);

private slots:
    /**
     * @brief Add a new DPI signature from user input.
     */
    void onAddSignature();

    /**
     * @brief Remove the currently selected DPI signature.
     */
    void onRemoveSignature();

    /**
     * @brief Test DPI detection on the provided payload.
     */
    void onTestDPI();

    /**
     * @brief Refresh the displayed list of signatures.
     */
    void refreshSignatureList();

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