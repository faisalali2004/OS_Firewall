#pragma once

#include <QWidget>
#include <QColor>

class QComboBox;
class QPushButton;
class QSpinBox;
class QLabel;
class TrafficShaper;

class TrafficShaperUI : public QWidget {
    Q_OBJECT
public:
    explicit TrafficShaperUI(QWidget* parent = nullptr);

private slots:
    void onApplyClicked();
    void onClearClicked();
    void refreshInterfaces();

private:
    void setStatus(const QString& text, QColor color = Qt::black);
    void setBusy(bool busy);

    QComboBox* ifaceBox;
    QPushButton* refreshBtn;
    QSpinBox* rateSpin;
    QSpinBox* burstSpin;
    QSpinBox* latencySpin;
    QPushButton* applyBtn;
    QPushButton* clearBtn;
    QLabel* statusLabel;
    QLabel* busyLabel;
    TrafficShaper* shaper;
};