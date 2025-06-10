#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QJsonArray>

class RuleEditor : public QWidget {
    Q_OBJECT
public:
    explicit RuleEditor(QWidget* parent = nullptr);
    void loadRules(const QString& path);
    void saveRules(const QString& path);

private slots:
    void addRule();
    void removeSelectedRule();
    void saveToFile();

private:
    QTableWidget* table;
    QPushButton* addBtn;
    QPushButton* removeBtn;
    QPushButton* saveBtn;
    QString rulesPath;
    void populateTable(const QJsonArray& rules);
    QJsonArray collectRules() const;
};