#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include "customplot/qcustomplot.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void on_open_btn_clicked();
    void on_close_btn_clicked();
    void Read_Date();       //读取串口数据

    void on_recv_text_clicked();

    void on_send_text_clicked();

    void on_send_data_clicked();

    void on_send_clear_clicked();

    void on_recv_clear_clicked();

private:
    void GetAveriablePort();
    void msleep(int msec);
    void PortConfigureInit();
    void setupQuadraticDemo(QCustomPlot *customPlot);

private:
    Ui::MainWindow *ui;
    QSerialPort *serialport;
    bool textstate_receive;
    bool textstate_send;
};
#endif // MAINWINDOW_H
