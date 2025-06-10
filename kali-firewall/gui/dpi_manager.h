#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <vector>
#include <QString>

struct GuiDPISignature {
    QString name;
    QString pattern;
    QString result;
};

class DPIManager : public QWidget {
    Q_OBJECT

public:
    explicit DPIManager(QWidget* parent = nullptr);
    ~DPIManager();

private slots:
    void addSignature();
    void removeSelectedSignature();

private:
    QTableWidget* table;
    QLineEdit* nameEdit;
    QLineEdit* patternEdit;
    QLineEdit* resultEdit;
    QPushButton* addBtn;
    QPushButton* removeBtn;
    QLabel* statusLabel;

    std::vector<GuiDPISignature> signatures;

    void updateStatus(const QString& msg, bool error = false);
    void populateTable();
};