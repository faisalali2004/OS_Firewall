#include "rule_editor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QFile>
#include <QJsonDocument>
#include <QMessageBox>

RuleEditor::RuleEditor(QWidget* parent)
    : QWidget(parent),
      table(new QTableWidget(this)),
      addBtn(new QPushButton("Add Rule", this)),
      removeBtn(new QPushButton("Remove Selected", this)),
      saveBtn(new QPushButton("Save", this)),
      loadBtn(new QPushButton("Load", this)),
      statusLabel(new QLabel(this)),
      rulesPath("../config/default_rules.json")
{
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"Source IP", "Destination IP", "Source Port", "Destination Port", "Action"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);

    auto* btnLayout = new QHBoxLayout;
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(removeBtn);
    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(loadBtn);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(table);
    mainLayout->addLayout(btnLayout);
    mainLayout->addWidget(statusLabel);

    setLayout(mainLayout);

    connect(addBtn, &QPushButton::clicked, this, &RuleEditor::addRule);
    connect(removeBtn, &QPushButton::clicked, this, &RuleEditor::removeSelectedRule);
    connect(saveBtn, &QPushButton::clicked, this, &RuleEditor::saveToFile);
    connect(loadBtn, &QPushButton::clicked, this, &RuleEditor::loadFromFile);

    loadRules(rulesPath);
    updateStatus("Ready.");
}

RuleEditor::~RuleEditor() = default;

void RuleEditor::addRule() {
    int row = table->rowCount();
    table->insertRow(row);
    for (int col = 0; col < table->columnCount(); ++col) {
        table->setItem(row, col, new QTableWidgetItem(""));
    }
    updateStatus("Added new rule.");
}

void RuleEditor::removeSelectedRule() {
    auto selected = table->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        updateStatus("No rule selected.", true);
        return;
    }
    table->removeRow(selected.first().row());
    updateStatus("Removed selected rule.");
}

void RuleEditor::saveToFile() {
    QString path = QFileDialog::getSaveFileName(this, "Save Rules", rulesPath, "JSON Files (*.json)");
    if (path.isEmpty()) return;
    saveRules(path);
}

void RuleEditor::loadFromFile() {
    QString path = QFileDialog::getOpenFileName(this, "Load Rules", rulesPath, "JSON Files (*.json)");
    if (path.isEmpty()) return;
    loadRules(path);
}

void RuleEditor::loadRules(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        updateStatus("Failed to open rule file: " + path, true);
        return;
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        updateStatus("Failed to parse JSON: " + err.errorString(), true);
        return;
    }
    if (!doc.isArray()) {
        updateStatus("Rule file format error.", true);
        return;
    }
    populateTable(doc.array());
    updateStatus("Loaded rules from: " + path);
}

void RuleEditor::saveRules(const QString& path) {
    QJsonArray rules = collectRules();
    QJsonDocument doc(rules);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        updateStatus("Failed to open file for writing: " + path, true);
        return;
    }
    file.write(doc.toJson());
    file.close();
    updateStatus("Saved rules to: " + path);
    emit rulesChanged(rules);
}

void RuleEditor::populateTable(const QJsonArray& rules) {
    table->setRowCount(0);
    for (const auto& val : rules) {
        if (!val.isObject()) continue;
        QJsonObject obj = val.toObject();
        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(obj.value("src_ip").toString()));
        table->setItem(row, 1, new QTableWidgetItem(obj.value("dst_ip").toString()));
        table->setItem(row, 2, new QTableWidgetItem(obj.value("src_port").toVariant().toString()));
        table->setItem(row, 3, new QTableWidgetItem(obj.value("dst_port").toVariant().toString()));
        table->setItem(row, 4, new QTableWidgetItem(obj.value("action").toString()));
    }
}

QJsonArray RuleEditor::collectRules() const {
    QJsonArray arr;
    for (int row = 0; row < table->rowCount(); ++row) {
        QJsonObject obj;
        obj["src_ip"] = table->item(row, 0) ? table->item(row, 0)->text() : "";
        obj["dst_ip"] = table->item(row, 1) ? table->item(row, 1)->text() : "";
        obj["src_port"] = table->item(row, 2) ? table->item(row, 2)->text().toInt() : 0;
        obj["dst_port"] = table->item(row, 3) ? table->item(row, 3)->text().toInt() : 0;
        obj["action"] = table->item(row, 4) ? table->item(row, 4)->text() : "";
        arr.append(obj);
    }
    return arr;
}

void RuleEditor::updateStatus(const QString& msg, bool error) {
    statusLabel->setText(msg);
    QPalette pal = statusLabel->palette();
    pal.setColor(QPalette::WindowText, error ? Qt::red : Qt::darkGreen);
    statusLabel->setPalette(pal);
}