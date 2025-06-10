#pragma once
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QMessageBox>

class TrafficShaperUI : public QWidget {
    Q_OBJECT

public:
    explicit TrafficShaperUI(QWidget* parent = nullptr);
    ~TrafficShaperUI();

signals:
    void shapingApplied(const QString& iface, int rate);
    void shapingCleared(const QString& iface);

private slots:
    void applyShaping();
    void clearShaping();

private:
    QComboBox* ifaceCombo;
    QLineEdit* rateEdit;
    QPushButton* applyBtn;
    QPushButton* clearBtn;
    QLabel* statusLabel;

    void updateStatus(const QString& msg, bool error = false);
    bool validateInput(QString& iface, int& rate);
    void populateInterfaces();
};