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

    serialPort.setPortName("/dev/pts/0");
    if(!serialPort.setBaudRate(QSerialPort::Baud115200))
        qDebug() << serialPort.errorString();
    if(!serialPort.setDataBits(QSerialPort::Data7))
        qDebug() << serialPort.errorString();
    if(!serialPort.setParity(QSerialPort::EvenParity))
        qDebug() << serialPort.errorString();
    if(!serialPort.setFlowControl(QSerialPort::HardwareControl))
        qDebug() << serialPort.errorString();
    if(!serialPort.setStopBits(QSerialPort::OneStop))
        qDebug() << serialPort.errorString();
    if(!serialPort.open(QIODevice::ReadOnly))
        qDebug() << serialPort.errorString();
    QObject::connect(&serialPort, &QSerialPort::readyRead, [&]
    {
        //this is called when readyRead() is emitted
        qDebug() << "New data available: " << serialPort.bytesAvailable();
        QByteArray datas = serialPort.readAll();
        qDebug() << datas;
    });
    QObject::connect(&serialPort,
                     static_cast<void(QSerialPort::*)(QSerialPort::SerialPortError)>
                     (&QSerialPort::error),
                     [&](QSerialPort::SerialPortError error)
    {
        //this is called when a serial communication error occurs
        qDebug() << "An error occured: " << error;
        telemetryApp.quit();
    });

    rootWindow.show();              // makes widgets visible
    return telemetryApp.exec();     // application event loop
}
