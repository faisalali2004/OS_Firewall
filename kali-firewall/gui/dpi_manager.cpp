#include "dpi_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>

DPIManager::DPIManager(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);

    table = new QTableWidget(this);
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels({"Name", "Pattern"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(table);

    auto* inputLayout = new QHBoxLayout();
    nameEdit = new QLineEdit(this);
    nameEdit->setPlaceholderText("Signature Name");
    patternEdit = new QLineEdit(this);
    patternEdit->setPlaceholderText("Regex Pattern");
    addBtn = new QPushButton("Add", this);
    removeBtn = new QPushButton("Remove Selected", this);

    inputLayout->addWidget(nameEdit);
    inputLayout->addWidget(patternEdit);
    inputLayout->addWidget(addBtn);
    inputLayout->addWidget(removeBtn);

    layout->addLayout(inputLayout);

    connect(addBtn, &QPushButton::clicked, this, &DPIManager::addSignature);
    connect(removeBtn, &QPushButton::clicked, this, &DPIManager::removeSelectedSignature);
}

void DPIManager::addSignature() {
    int row = table->rowCount();
    table->insertRow(row);
    table->setItem(row, 0, new QTableWidgetItem(nameEdit->text()));
    table->setItem(row, 1, new QTableWidgetItem(patternEdit->text()));
    nameEdit->clear();
    patternEdit->clear();
}

void DPIManager::removeSelectedSignature() {
    auto rows = table->selectionModel()->selectedRows();
    for (const auto& idx : rows)
        table->removeRow(idx.row());
}