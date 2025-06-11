#include <QApplication>
#include <QMetaType>
#include "mainwindow.h"
#include "core/packet_info.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qRegisterMetaType<PacketInfo>("PacketInfo");

    MainWindow w;
    w.show();

    return app.exec();
}