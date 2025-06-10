#pragma once
#include <QWidget>

class LogViewer : public QWidget {
    Q_OBJECT

public:
    explicit LogViewer(QWidget* parent = nullptr);
    ~LogViewer();
    // ... other members ...
};