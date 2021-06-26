#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include <QLabel>
#include <QMessageBox>
#include <QTime>
#include <QElapsedTimer>
#include "conf.h"
#include <QList>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_status(new QLabel),
    m_settings(new SettingsDialog),
    m_serial(new QSerialPort(this))
{
    m_ui->setupUi(this);
    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionConfigure->setEnabled(true);
    m_ui->statusBar->addWidget(m_status);
    m_ui->customPlot->addGraph();
    m_ui->customPlot->addGraph(m_ui->customPlot->xAxis, m_ui->customPlot->yAxis2);
    m_ui->customPlot->graph(0)->setPen(QPen(QColor(40, 110, 255))); // blue line
    m_ui->customPlot->graph(1)->setPen(QPen(QColor(255, 110, 40))); // red line
    m_ui->customPlot->yAxis->setRange(0, 180);
    m_ui->customPlot->yAxis2->setRange(-50, 50);

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    m_ui->customPlot->xAxis->setTicker(timeTicker);
    timeTicker->setTimeFormat("%h:%m:%s");
    //m_ui->customPlot->axisRect()->setupFullAxesBox();

    connect(m_ui->pushButton,SIGNAL(clicked()),this,SLOT(TogglePushButton()));
    connect(m_ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(m_ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(m_ui->actionConfigure, &QAction::triggered, m_settings, &SettingsDialog::show);
    connect(m_serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError);
    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::readData);

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    //mainTimer.callOnTimeout(SLOT(realtimeDataSlot()));
    connect(&mainTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    mainTimer.start(PLOT_REFRESH_TIME); // Interval 0 means to refresh as fast as possible

    elapsedTimer.start();
}

void MainWindow::openSerialPort()
{
    const SettingsDialog::Settings p = m_settings->settings();
    m_serial->setPortName(p.name);
    m_serial->setBaudRate(p.baudRate);
    m_serial->setDataBits(p.dataBits);
    m_serial->setParity(p.parity);
    m_serial->setStopBits(p.stopBits);
    m_serial->setFlowControl(p.flowControl);

    if (m_serial->open(QIODevice::ReadWrite))
    {
        m_ui->actionConnect->setEnabled(false);
        m_ui->actionDisconnect->setEnabled(true);
        m_ui->actionConfigure->setEnabled(false);
        showStatusMessage(tr("Connected to %1").arg("Self-balancing robot"));
    } else {
        QMessageBox::critical(this, tr("Error"), m_serial->errorString());
        showStatusMessage(tr("Open error"));
    }
}

void MainWindow::closeSerialPort()
{
    if (m_serial->isOpen()) {
        m_serial->close();
    }
    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionConfigure->setEnabled(true);
    showStatusMessage(tr("Disconnected"));
    robotAngle = 0;
    pidError = 0;
    m_ui->label_Kp_value->setText("N/A");
    m_ui->label_Ki_value->setText("N/A");
    m_ui->label_Kd_value->setText("N/A");
}

void MainWindow::readData()
{
    QByteArray byteArr;
    QList<QByteArray> signalsPtr;

    byteArr = m_serial->readAll();

    if(!(byteArr.front() != '!' || byteArr.size() < 8)) {
        signalsPtr = byteArr.split('\t');
    }
    if(signalsPtr.size() == 2) {
        robotAngle = (signalsPtr[0].right(signalsPtr[0].size()-1)).toFloat();
        pidError = signalsPtr[1].toFloat();
    } else if (signalsPtr.size() == 6) {
        robotAngle = (signalsPtr[0].right(signalsPtr[0].size()-1)).toFloat();
        pidError = signalsPtr[1].toFloat();
        m_ui->label_Kp_value->setText(signalsPtr[2]);
        m_ui->label_Ki_value->setText(signalsPtr[3]);
        m_ui->label_Kd_value->setText(signalsPtr[4]);
/*
        qDebug() << byteArr << signalsPtr.size();
        for (int i=0; i< signalsPtr.size(); i++) {
            qDebug() << signalsPtr[i];
        }
        qDebug() << "\r\n";
*/
    }
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), m_serial->errorString());
        closeSerialPort();
    }
}

void MainWindow::TogglePushButton() {
    pushButtonState = !pushButtonState;
    qDebug() << pushButtonState;
}

void MainWindow::showStatusMessage(const QString &message) {
    m_status->setText(message);
}

void MainWindow::realtimeDataSlot() {
    double elapsedTime = elapsedTimer.elapsed(); // time elapsed since start of demo, in milliseconds
    m_ui->customPlot->graph(0)->addData(elapsedTime, robotAngle);
    m_ui->customPlot->graph(1)->addData(elapsedTime, pidError);
    // make key axis range scroll with the data (at a constant range size of 8):
    m_ui->customPlot->xAxis->setRange(elapsedTime, 5000, Qt::AlignRight);
    m_ui->customPlot->replot();
}

MainWindow::~MainWindow()
{
    delete m_ui;
}
