#pragma once
#include <QString>

struct PacketInfo {
    QString srcIp;
    QString dstIp;
    int srcPort;
    int dstPort;
    // Add more fields if needed
};
Q_DECLARE_METATYPE(PacketInfo)