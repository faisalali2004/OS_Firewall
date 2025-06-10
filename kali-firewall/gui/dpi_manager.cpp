#include "dpi_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>

DPIManager::DPIManager(QWidget* parent)
    : QWidget(parent),
      table(new QTableWidget(this)),
      nameEdit(new QLineEdit(this)),
      patternEdit(new QLineEdit(this)),
      resultEdit(new QLineEdit(this)),
      addBtn(new QPushButton("Add Signature", this)),
      removeBtn(new QPushButton("Remove Selected", this)),
      statusLabel(new QLabel(this))
{
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({"Name", "Pattern", "Result"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    nameEdit->setPlaceholderText("Signature Name");
    patternEdit->setPlaceholderText("Regex Pattern");
    resultEdit->setPlaceholderText("Result Label");

    auto* inputLayout = new QHBoxLayout;
    inputLayout->addWidget(nameEdit);
    inputLayout->addWidget(patternEdit);
    inputLayout->addWidget(resultEdit);
    inputLayout->addWidget(addBtn);

    auto* btnLayout = new QHBoxLayout;
    btnLayout->addWidget(removeBtn);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(table);
    mainLayout->addLayout(inputLayout);
    mainLayout->addLayout(btnLayout);
    mainLayout->addWidget(statusLabel);

    setLayout(mainLayout);

    connect(addBtn, &QPushButton::clicked, this, &DPIManager::addSignature);
    connect(removeBtn, &QPushButton::clicked, this, &DPIManager::removeSelectedSignature);

    // Optionally, add some default signatures
    signatures.push_back({"HTTP", R"(^GET |^POST |^HTTP/1\.)", "HTTP"});
    signatures.push_back({"DNS", R"(^.{2}\x01\x00)", "DNS"});
    signatures.push_back({"TLS", R"(^\x16\x03)", "TLS"});
    signatures.push_back({"SSH", R"(^SSH-)", "SSH"});
    populateTable();
    updateStatus("Ready.");
}

DPIManager::~DPIManager() = default;

void DPIManager::addSignature() {
    QString name = nameEdit->text().trimmed();
    QString pattern = patternEdit->text().trimmed();
    QString result = resultEdit->text().trimmed();

    if (name.isEmpty() || pattern.isEmpty() || result.isEmpty()) {
        updateStatus("All fields are required.", true);
        return;
    }

    // Optionally, validate regex
    QRegExp rx(pattern);
    if (!rx.isValid()) {
        updateStatus("Invalid regex pattern.", true);
        return;
    }

    signatures.push_back({name, pattern, result});
    populateTable();
    nameEdit->clear();
    patternEdit->clear();
    resultEdit->clear();
    updateStatus("Signature added.");
}

void DPIManager::removeSelectedSignature() {
    auto selected = table->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        updateStatus("No signature selected.", true);
        return;
    }
    int row = selected.first().row();
    if (row >= 0 && row < static_cast<int>(signatures.size())) {
        signatures.erase(signatures.begin() + row);
        populateTable();
        updateStatus("Signature removed.");
    }
}

void DPIManager::populateTable() {
    table->setRowCount(0);
    for (const auto& sig : signatures) {
        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(sig.name));
        table->setItem(row, 1, new QTableWidgetItem(sig.pattern));
        table->setItem(row, 2, new QTableWidgetItem(sig.result));
    }
}

void DPIManager::updateStatus(const QString& msg, bool error) {
    statusLabel->setText(msg);
    QPalette pal = statusLabel->palette();
    pal.setColor(QPalette::WindowText, error ? Qt::red : Qt::darkGreen);
    statusLabel->setPalette(pal);
}