#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

class DPIManager : public QWidget {
    Q_OBJECT
public:
    explicit DPIManager(QWidget* parent = nullptr);

private slots:
    void addSignature();
    void removeSelectedSignature();

private:
    QTableWidget* table;
    QLineEdit* nameEdit;
    QLineEdit* patternEdit;
    QPushButton* addBtn;
    QPushButton* removeBtn;
};