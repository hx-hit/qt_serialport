#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include "customplot/qcustomplot.h"
#include "QTimer"
#include "RingBuffer.hpp"
#include "iostream"
#include "mutex"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

typedef struct frame_{
    uint8_t head;
    uint8_t type;
    uint8_t status;
    uint8_t work_mode;
    uint8_t value[2];
    uint8_t check;
}frame;


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
    void realtimeDataSlot();
    void updateLabel();

private:
    void GetAveriablePort();
    void msleep(int msec);
    void PortConfigureInit();
    void setupQuadraticDemo(QCustomPlot *customPlot);

    static uint8_t checkData(uint8_t* input, int size);
    static bool checkData(const frame& frame);
    static void convert(const QByteArray& buf, int size, frame* frame);

private:
    Ui::MainWindow *ui;
    QSerialPort *serialport;
    bool textstate_receive;
    bool textstate_send;
    frame mFrame;
    QTimer dataTimer;
    QTimer labelTimer;
    std::mutex data_mut;
    uint16_t mValues[5];
};
#endif // MAINWINDOW_H
