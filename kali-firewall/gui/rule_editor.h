#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QJsonArray>

class RuleEditor : public QWidget {
    Q_OBJECT

public:
    explicit RuleEditor(QWidget* parent = nullptr);
    ~RuleEditor();
    // ... other members ...
};