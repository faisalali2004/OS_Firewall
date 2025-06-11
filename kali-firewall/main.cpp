#include <QApplication>
#include "mainwindow.h" // Make sure this matches your main window class header

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow w;
    w.show();

    return app.exec();
}