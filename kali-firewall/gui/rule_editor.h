#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QJsonArray>

class RuleEditor : public QWidget {
    Q_OBJECT
public:
    explicit RuleEditor(QWidget* parent = nullptr);
    ~RuleEditor();

    // Loads rules from a file
    void loadRules(const QString& path);

    // Saves rules to a file
    void saveRules(const QString& path);

    // Validates all rules in the table
    bool validateAllRules() const;

signals:
    void rulesChanged(const QJsonArray& rules);

private slots:
    void addRule();
    void removeSelectedRule();
    void saveToFile();
    void loadFromFile();

private:
    void populateTable(const QJsonArray& rules);
    QJsonArray collectRules() const;
    void updateStatus(const QString& msg, bool error = false);

    QTableWidget* table;
    QPushButton* addBtn;
    QPushButton* removeBtn;
    QPushButton* saveBtn;
    QPushButton* loadBtn;
    QLabel* statusLabel;
    QString rulesPath;
};