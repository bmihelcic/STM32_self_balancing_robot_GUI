#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort>

QT_BEGIN_NAMESPACE
class QLabel;
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Console;
class SettingsDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    bool pushButtonState = false;
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();    

private:
    QElapsedTimer elapsedTimer;
    float robotAngle = 0;
    float pidError = 0;
    QTimer mainTimer;
    Ui::MainWindow *m_ui = nullptr;
    QLabel *m_status = nullptr;
    Console *m_console = nullptr;
    SettingsDialog *m_settings = nullptr;
    QSerialPort *m_serial = nullptr;

    void showStatusMessage(const QString &message);

private slots:
    void openSerialPort();
    void closeSerialPort();
    void ToggleStartEnginePushButton();
    void ClickKpPlus();
    void ClickKpPlus10();
    void ClickKpMinus();
    void ClickKpMinus10();
    void ClickKiPlus();
    void ClickKiPlus1();
    void ClickKiMinus();
    void ClickKiMinus1();
    void ClickKdPlus();
    void ClickKdPlus10();
    void ClickKdMinus();
    void ClickKdMinus10();
    void readData();
    void handleError(QSerialPort::SerialPortError error);
    void realtimeDataSlot();
};
#endif // MAINWINDOW_H
