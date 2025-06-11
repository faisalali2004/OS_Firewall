#include <QApplication>
#include <QMetaType>
#include "mainwindow.h"
#include "packet_info.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Register PacketInfo for queued signal/slot connections
    qRegisterMetaType<PacketInfo>("PacketInfo");

    MainWindow w;
    w.show();

    return app.exec();
}