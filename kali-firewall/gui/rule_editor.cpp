#include "rule_editor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHeaderView>
#include <QMessageBox>

RuleEditor::RuleEditor(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    table = new QTableWidget(this);
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({"Src IP", "Src Port", "Dst IP", "Dst Port", "Protocol", "Action"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(table);

    auto* btnLayout = new QHBoxLayout();
    addBtn = new QPushButton("Add Rule", this);
    removeBtn = new QPushButton("Remove Selected", this);
    saveBtn = new QPushButton("Save", this);
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(removeBtn);
    btnLayout->addWidget(saveBtn);
    layout->addLayout(btnLayout);

    connect(addBtn, &QPushButton::clicked, this, &RuleEditor::addRule);
    connect(removeBtn, &QPushButton::clicked, this, &RuleEditor::removeSelectedRule);
    connect(saveBtn, &QPushButton::clicked, this, &RuleEditor::saveToFile);

    rulesPath = "../config/default_rules.json";
    loadRules(rulesPath);
}

void RuleEditor::loadRules(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return;
    QByteArray data = file.readAll();
    file.close();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) return;
    populateTable(doc.array());
}

void RuleEditor::saveRules(const QString& path) {
    QJsonArray arr = collectRules();
    QJsonDocument doc(arr);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return;
    file.write(doc.toJson());
    file.close();
}

void RuleEditor::addRule() {
    int row = table->rowCount();
    table->insertRow(row);
    for (int col = 0; col < table->columnCount(); ++col)
        table->setItem(row, col, new QTableWidgetItem(""));
}

void RuleEditor::removeSelectedRule() {
    auto rows = table->selectionModel()->selectedRows();
    for (const auto& idx : rows)
        table->removeRow(idx.row());
}

void RuleEditor::saveToFile() {
    saveRules(rulesPath);
    QMessageBox::information(this, "Saved", "Rules saved to file.");
}

void RuleEditor::populateTable(const QJsonArray& rules) {
    table->setRowCount(0);
    for (const auto& v : rules) {
        QJsonObject obj = v.toObject();
        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(obj.value("src_ip").toString()));
        table->setItem(row, 1, new QTableWidgetItem(QString::number(obj.value("src_port").toInt())));
        table->setItem(row, 2, new QTableWidgetItem(obj.value("dst_ip").toString()));
        table->setItem(row, 3, new QTableWidgetItem(QString::number(obj.value("dst_port").toInt())));
        table->setItem(row, 4, new QTableWidgetItem(obj.value("protocol").toString()));
        table->setItem(row, 5, new QTableWidgetItem(obj.value("action").toString()));
    }
}

QJsonArray RuleEditor::collectRules() const {
    QJsonArray arr;
    for (int row = 0; row < table->rowCount(); ++row) {
        QJsonObject obj;
        obj["src_ip"] = table->item(row, 0) ? table->item(row, 0)->text() : "";
        obj["src_port"] = table->item(row, 1) ? table->item(row, 1)->text().toInt() : 0;
        obj["dst_ip"] = table->item(row, 2) ? table->item(row, 2)->text() : "";
        obj["dst_port"] = table->item(row, 3) ? table->item(row, 3)->text().toInt() : 0;
        obj["protocol"] = table->item(row, 4) ? table->item(row, 4)->text() : "";
        obj["action"] = table->item(row, 5) ? table->item(row, 5)->text() : "";
        arr.append(obj);
    }
    return arr;
}