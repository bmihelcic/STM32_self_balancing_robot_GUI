#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include <QLabel>
#include <QMessageBox>
#include <QTime>
#include <QElapsedTimer>

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
    m_ui->customPlot->graph(0)->setPen(QPen(QColor(40, 110, 255)));
    //m_ui->customPlot->xAxis->setRange(0, 500);
    //m_ui->customPlot->yAxis->setRange(0, 100);


    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    m_ui->customPlot->xAxis->setTicker(timeTicker);
    //m_ui->customPlot->axisRect()->setupFullAxesBox();


    connect(m_ui->pushButton,SIGNAL(clicked()),this,SLOT(TogglePushButton()));
    connect(m_ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(m_ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(m_ui->actionConfigure, &QAction::triggered, m_settings, &SettingsDialog::show);
    connect(m_serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError);
    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::readData);


    static QTimer mainTimer;
    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    //mainTimer.callOnTimeout(SLOT(realtimeDataSlot()));
    connect(&mainTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    mainTimer.start(0); // Interval 0 means to refresh as fast as possible

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
        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
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
}

void MainWindow::readData()
{
    static QVector<double> x, y; // initialize with entries 0..100
    static int i= 0;
    char data[10];
    QString string = data;
    float num;
    m_serial->readLine(data,10);
    num = string.toFloat();
    if(num != 0) {
        i++;
        x.append(i);
        y.append(num);
        qDebug() << num;
        m_ui->customPlot->graph(0)->addData(x, y);
        m_ui->customPlot->replot();
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
    // calculate two new data points:
    double key = elapsedTimer.elapsed(); // time elapsed since start of demo, in milliseconds
    static double lastPointKey = 0;
    if (key-lastPointKey > 50) // at most add point every 70 ms
    {
      // add data to lines:
      m_ui->customPlot->graph(0)->addData(key, qSin(key));
      // rescale value (vertical) axis to fit the current data:
      m_ui->customPlot->graph(0)->rescaleValueAxis();
      lastPointKey = key;

    // make key axis range scroll with the data (at a constant range size of 8):
    m_ui->customPlot->xAxis->setRange(key, 5000, Qt::AlignRight);
    m_ui->customPlot->replot();
}
    // calculate frames per second:
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 2) // average fps over 2 seconds
    {
      showStatusMessage(
            tr("%1 FPS, Total Data points: %2")
            .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
            .arg(m_ui->customPlot->graph(0)->data()->size()));
      lastFpsKey = key;
      frameCount = 0;
    }
}

MainWindow::~MainWindow()
{
    delete m_ui;
}
