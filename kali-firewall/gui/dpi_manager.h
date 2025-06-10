#pragma once
#include <QWidget>

class DPIManager : public QWidget {
    Q_OBJECT

public:
    explicit DPIManager(QWidget* parent = nullptr);
    ~DPIManager();
    // ... other members ...
};