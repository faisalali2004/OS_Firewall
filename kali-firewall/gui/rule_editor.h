#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QJsonArray>
#include <QLabel>

class RuleEditor : public QWidget {
    Q_OBJECT

public:
    explicit RuleEditor(QWidget* parent = nullptr);
    ~RuleEditor();

signals:
    void rulesChanged(const QJsonArray& rules);

private slots:
    void addRule();
    void removeSelectedRule();
    void saveToFile();
    void loadFromFile();

private:
    void loadRules(const QString& path);
    void saveRules(const QString& path);
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