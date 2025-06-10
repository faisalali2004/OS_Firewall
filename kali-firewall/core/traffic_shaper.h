#pragma once

#include <QObject>
#include <QString>

/**
 * @brief The TrafficShaper class provides asynchronous bandwidth shaping and clearing
 *        for a given network interface using Linux 'tc' (traffic control).
 *        Emits detailed signals for GUI integration and error handling.
 */
class TrafficShaper : public QObject {
    Q_OBJECT
public:
    explicit TrafficShaper(QObject* parent = nullptr);

    /**
     * @brief Asynchronously shape bandwidth on a given interface.
     * @param iface Interface name (e.g., "eth0")
     * @param rate_kbit Bandwidth limit in kbit/s
     * @param burst_kbit Burst size in kbit (default 32)
     * @param latency_ms Latency in ms (default 400)
     */
    void shape(const QString& iface, int rate_kbit, int burst_kbit = 32, int latency_ms = 400);

    /**
     * @brief Asynchronously remove shaping from an interface.
     * @param iface Interface name (e.g., "eth0")
     */
    void clear(const QString& iface);

signals:
    void shapingSuccess(const QString& iface);
    void shapingError(const QString& iface, const QString& error);
    void cleared(const QString& iface);
    void clearError(const QString& iface, const QString& error);
};