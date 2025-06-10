#include "traffic_shaper.h"
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QRegularExpression>

/*
 * TrafficShaper: Asynchronous, robust Linux traffic shaping using tc.
 * Features:
 *  - Emits detailed error messages (stderr/stdout from tc).
 *  - Checks for root privileges.
 *  - Checks if the interface exists before shaping.
 *  - Validates parameters.
 *  - Non-blocking, safe for GUI use.
 */

TrafficShaper::TrafficShaper(QObject* parent)
    : QObject(parent)
{}

/**
 * @brief Checks if the current process has root privileges.
 * @return true if root, false otherwise.
 */
static bool isRoot() {
    return (geteuid() == 0);
}

/**
 * @brief Checks if a network interface exists.
 * @param iface Interface name (e.g., "eth0")
 * @return true if exists, false otherwise.
 */
static bool interfaceExists(const QString& iface) {
    QFile file("/sys/class/net/" + iface);
    return file.exists();
}

/**
 * @brief Asynchronously shape bandwidth on a given interface.
 * @param iface Interface name (e.g., "eth0")
 * @param rate_kbit Bandwidth limit in kbit/s
 * @param burst_kbit Burst size in kbit (default 32)
 * @param latency_ms Latency in ms (default 400)
 */
void TrafficShaper::shape(const QString& iface, int rate_kbit, int burst_kbit, int latency_ms) {
    // --- Parameter validation ---
    if (iface.isEmpty() || !interfaceExists(iface)) {
        emit shapingError(iface, "Interface does not exist: " + iface);
        return;
    }
    if (rate_kbit <= 0 || burst_kbit <= 0 || latency_ms < 0) {
        emit shapingError(iface, "Invalid parameters: rate, burst, and latency must be positive.");
        return;
    }
    if (!isRoot()) {
        emit shapingError(iface, "Root privileges required to run tc.");
        return;
    }

    // --- Remove any existing qdisc first (async) ---
    QProcess* clearProc = new QProcess(this);
    connect(clearProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int, QProcess::ExitStatus){
        clearProc->deleteLater();

        // --- Add root qdisc ---
        QProcess* rootProc = new QProcess(this);
        QString rootCmd = QString("tc qdisc add dev %1 root handle 1: htb default 10").arg(iface);
        connect(rootProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [=](int rootCode, QProcess::ExitStatus){
            QString stdErr = rootProc->readAllStandardError();
            QString stdOut = rootProc->readAllStandardOutput();
            rootProc->deleteLater();
            if (rootCode != 0) {
                emit shapingError(iface, "Failed to add root qdisc:\n" + stdErr + stdOut);
                return;
            }

            // --- Add class for shaping ---
            QProcess* classProc = new QProcess(this);
            QString classCmd = QString("tc class add dev %1 parent 1: classid 1:10 htb rate %2kbit burst %3kbit")
                                .arg(iface).arg(rate_kbit).arg(burst_kbit);
            connect(classProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                    this, [=](int classCode, QProcess::ExitStatus){
                QString stdErr = classProc->readAllStandardError();
                QString stdOut = classProc->readAllStandardOutput();
                classProc->deleteLater();
                if (classCode != 0) {
                    emit shapingError(iface, "Failed to add class:\n" + stdErr + stdOut);
                    clear(iface);
                    return;
                }

                // --- Add netem for latency ---
                QProcess* netemProc = new QProcess(this);
                QString netemCmd = QString("tc qdisc add dev %1 parent 1:10 handle 10: netem delay %2ms")
                                    .arg(iface).arg(latency_ms);
                connect(netemProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                        this, [=](int netemCode, QProcess::ExitStatus){
                    QString stdErr = netemProc->readAllStandardError();
                    QString stdOut = netemProc->readAllStandardOutput();
                    netemProc->deleteLater();
                    if (netemCode != 0) {
                        emit shapingError(iface, "Failed to add netem:\n" + stdErr + stdOut);
                        clear(iface);
                        return;
                    }
                    emit shapingSuccess(iface);
                });
                netemProc->start("bash", QStringList() << "-c" << netemCmd);
            });
            classProc->start("bash", QStringList() << "-c" << classCmd);
        });
        rootProc->start("bash", QStringList() << "-c" << rootCmd);
    });
    clearProc->start("bash", QStringList() << "-c" << QString("tc qdisc del dev %1 root").arg(iface));
}

/**
 * @brief Asynchronously remove shaping from an interface.
 * @param iface Interface name (e.g., "eth0")
 */
void TrafficShaper::clear(const QString& iface) {
    if (iface.isEmpty() || !interfaceExists(iface)) {
        emit clearError(iface, "Interface does not exist: " + iface);
        return;
    }
    if (!isRoot()) {
        emit clearError(iface, "Root privileges required to run tc.");
        return;
    }
    QProcess* proc = new QProcess(this);
    QString cmd = QString("tc qdisc del dev %1 root").arg(iface);
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int code, QProcess::ExitStatus){
        QString stdErr = proc->readAllStandardError();
        QString stdOut = proc->readAllStandardOutput();
        proc->deleteLater();
        if (code == 0) {
            emit cleared(iface);
        } else {
            emit clearError(iface, "Failed to clear shaping (maybe already cleared):\n" + stdErr + stdOut);
        }
    });
    proc->start("bash", QStringList() << "-c" << cmd);
}