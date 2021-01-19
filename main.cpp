#include "mainwindow.h"
#include "QDebug"
#include <QApplication>
#include <QSerialPort>
#include <QSerialPortInfo>

int main(int argc, char *argv[])
{
    QApplication telemetryApp(argc, argv);
    MainWindow rootWindow;
    QSerialPort serialPort;

    rootWindow.show();              // makes widgets visible
    return telemetryApp.exec();     // application event loop
}
