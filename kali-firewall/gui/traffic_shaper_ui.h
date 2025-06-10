#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class TrafficShaperUI : public QWidget {
    Q_OBJECT
public:
    explicit TrafficShaperUI(QWidget* parent = nullptr);

private slots:
    void applyShaping();
    void clearShaping();

private:
    QLineEdit* ifaceEdit;
    QLineEdit* rateEdit;
    QPushButton* applyBtn;
    QPushButton* clearBtn;
    QLabel* statusLabel;
};