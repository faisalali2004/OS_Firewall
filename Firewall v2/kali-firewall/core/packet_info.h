#pragma once
#include <QString>
#include <QMetaType>

struct PacketInfo {
    QString srcIp;
    QString dstIp;
    int srcPort;
    int dstPort;
    QString protocol;
};
Q_DECLARE_METATYPE(PacketInfo)