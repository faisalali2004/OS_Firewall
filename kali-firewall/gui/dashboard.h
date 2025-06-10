#pragma once

#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QTimer>

class Dashboard : public QWidget {
    Q_OBJECT
public:
    explicit Dashboard(QWidget* parent = nullptr);

private slots:
    void refreshStats();

private:
    QLabel* packetRateLabel;
    QLabel* dpiMatchLabel;
    QTableWidget* connTable;
    QTimer* timer;
    void loadConnections();
};