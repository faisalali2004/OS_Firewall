#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

class TrafficShaperUI : public QWidget {
    Q_OBJECT

public:
    explicit TrafficShaperUI(QWidget* parent = nullptr);
    ~TrafficShaperUI();
    // ... other members ...
};