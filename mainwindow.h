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
#include "condition_variable"
#include "thread"
#include "deque"
#include "map"
#include <QDateTime>
#include "savethread.h"
#include "flowrate.h"
#include "xlsxdocument.h"
#include "xlsxchartsheet.h"
#include "xlsxcellrange.h"
#include "xlsxchart.h"
#include "xlsxrichstring.h"
#include "xlsxworkbook.h"
using namespace QXlsx;

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

    void on_btn_stop_clicked();

    void on_btn_save_s1_clicked();

    void on_btn_save_t_po_clicked();

    void on_btn_save_r_po_clicked();

    void on_btn_save_t_pr_clicked();

    void on_btn_save_s2_clicked();

    void on_btn_save_all_clicked();

    void on_actionFlowRate_triggered();

private:
    void GetAveriablePort();
    void msleep(int msec);
    void PortConfigureInit();
    void setupQuadraticDemo(QCustomPlot *customPlot);
    void save_thread_function();
    void show_label_function();
    void draw_graphy_function();

    static uint8_t checkData(uint8_t* input, int size);
    static bool checkData(const frame& frame);
    static void convert(const QByteArray& buf, int size, frame* frame);

private:
    Ui::MainWindow *ui;
    QSerialPort *serialport;
    bool textstate_receive{false};
    bool textstate_send;
    bool stop_display;
    frame mFrame;
    QTimer dataTimer;
    QTimer labelTimer;
    std::mutex data_mut;
    uint16_t mValues[5];
    std::unique_ptr<std::thread> saveThreadPtr;
    std::deque<frame> frameQueue;
    bool save_flag{false};
    bool save_t_po{false};
    bool save_r_po{false};
    bool save_t_pr{false};
    bool save_r_s1{false};
    bool save_r_s2{false};
    bool save_all{false};
    std::mutex queue_mut;
    std::mutex cond_mut;
    std::condition_variable queue_cond;
    QXlsx::Document xlsx;
    std::map<uint8_t, std::deque<uint16_t>> saveDataMap;
    bool saveFlagVector[5]{false,false,false,false,false};
    QThreadPool *mPool;
    SaveThread *mSaveThread[5]{nullptr};
    int file_count{0};
    QXlsx::Document* m_xlsx = nullptr;
    QString m_saveName;
    std::thread *saveAllThread;
    std::thread *showLabel;
    std::thread *drawGraphy;
    FlowRate* mFlowRate{nullptr};
};
#endif // MAINWINDOW_H
