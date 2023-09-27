#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QTime"
#include "QMessageBox"
#include "QDebug"
#include "cmd.h"

RingBuffer<frame, 5> ringBuffer;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    for(int i=0;i<5;i++){
        mValues[i] = 0;
    }
    ui->setupUi(this);

    serialport = new QSerialPort;
    GetAveriablePort();
    PortConfigureInit();
    setupQuadraticDemo(ui->customplot);
    ui->customplot->replot();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::GetAveriablePort()
{
    ui->serial_num->clear();
    foreach (auto &info, QSerialPortInfo::availablePorts()) {
        QSerialPort serial;
        serial.setPort(info);
        if(serial.open(QIODevice::ReadWrite)){
            ui->serial_num->addItem(serial.portName());
            serial.close();
        }
    }
}

void MainWindow::msleep(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void MainWindow::PortConfigureInit()
{
    ui->bitrate->addItem("115200", "115200");
    ui->bitrate->addItem("38400", "38400");
    ui->bitrate->addItem("19200", "19200");
    ui->bitrate->addItem("9600", "9600");

    ui->data_bit->addItem("8", 8);
    ui->data_bit->addItem("7", 7);

    ui->check_bit->addItem("无校验", 0);

    ui->stop_bit->addItem("1位", 1);
    ui->stop_bit->addItem("2位", 2);
}


void MainWindow::on_open_btn_clicked()
{
    update();
    msleep(10);
    GetAveriablePort();
    serialport->setPortName(ui->serial_num->currentText());
    if(serialport->open(QIODevice::ReadWrite))
    {
        serialport->setBaudRate(ui->bitrate->currentText().toInt());
        switch (ui->data_bit->currentData().toInt()) {
        case 8:
            serialport->setDataBits(QSerialPort::Data8);
            break;
        case 7:
            serialport->setDataBits(QSerialPort::Data7);
            break;
        default:
            break;
        }
        switch (ui->check_bit->currentIndex()) {
        case 0:
            serialport->setParity(QSerialPort::NoParity);
            break;
        default:
            break;
        }
        switch (ui->stop_bit->currentIndex()) {
        case 0:
            serialport->setStopBits(QSerialPort::OneStop);
            break;
        case 1:
            serialport->setStopBits(QSerialPort::TwoStop);
            break;
        default:
            break;
        }
        serialport->setFlowControl(QSerialPort::NoFlowControl);
        connect(serialport, &QSerialPort::readyRead, this, &MainWindow::Read_Date);
        ui->send_data->setEnabled(true);
        ui->open_btn->setEnabled(false);

    }else{
        msleep(100);
        QMessageBox::information(this,tr("Erro"),tr("Open the failure"),QMessageBox::Ok);
    }
}


void MainWindow::on_close_btn_clicked()
{
    serialport->clear();        //清空缓存区
    serialport->close();        //关闭串口
    ui->send_data->setEnabled(false);
    ui->open_btn->setEnabled(true);
}

void MainWindow::Read_Date()
{
    static int cnt = 0;
    QByteArray buf;
    buf = serialport->readAll();

    if(!buf.isEmpty()){
        if(buf.size() != 7){
            cnt++;
            std::cout<<"single time recv len error total : "<<cnt<<std::endl;
        }else{
            data_mut.lock();
            convert(buf, 7, &mFrame);
            data_mut.unlock();
        }
        if(textstate_receive == true)   //文本模式
        {
            QString str = ui->recv_text_window->toPlainText();
            str += tr(buf);
            str += " ";
            ui->recv_text_window->clear();
            ui->recv_text_window->append(str);
        }
        if(textstate_receive == false)   //文本模式
        {
            QString str = ui->recv_text_window->toPlainText();
            if(str.size()>500){
                str = str.right(500);
            }
            // byteArray 转 16进制
            QByteArray temp = buf.toHex();
            str += tr(temp);
            str += "  ";
//            qDebug()<<str.size();
            ui->recv_text_window->clear();
            ui->recv_text_window->append(str);
        }
    }
    buf.clear();
}


void MainWindow::on_recv_text_clicked()
{
    if(ui->recv_text->text() == "文本模式"){
        textstate_receive = true;
        ui->recv_text->setText("hex模式");
    }
    else{
        ui->recv_text->setText("文本模式");
        textstate_receive = false;
    }
}


void MainWindow::on_send_text_clicked()
{
    if(ui->send_text->text() == "文本模式"){
        textstate_send = true;
        ui->send_text->setText("hex模式");
    }
    else{
        ui->send_text->setText("文本模式");
        textstate_send = false;
    }
}


void MainWindow::on_send_data_clicked()
{
    std::cout<<__func__<<std::endl;
    if(textstate_send == true)  //文版模式
    {
        serialport->write(ui->send_text_window->toPlainText().toLatin1());
    }

    if(textstate_send == false)     //16进制
    {
        QString str = ui->send_text_window->toPlainText();
        qDebug()<<str;
        int num = str.toInt();
        str = str.setNum(num,16);
        ui->send_text_window->clear();
        ui->send_text_window->append(str);
        serialport->write(ui->send_text_window->toPlainText().toLatin1());
    }
}


void MainWindow::on_send_clear_clicked()
{
    ui->send_text_window->clear();
}


void MainWindow::on_recv_clear_clicked()
{
    ui->recv_text_window->clear();
}

void MainWindow::setupQuadraticDemo(QCustomPlot *customPlot)
{
    customPlot->addGraph(); // blue line
    customPlot->graph(0)->setPen(QPen(QColor(40, 110, 255)));
    customPlot->graph(0)->setName("阀门目标位置");
    customPlot->addGraph(); // red line
    customPlot->graph(1)->setPen(QPen(QColor(255, 110, 40)));
    customPlot->graph(1)->setName("阀门实际位置");
    customPlot->addGraph(); // red line
    customPlot->graph(2)->setPen(QPen(QColor(40, 255, 40)));
    customPlot->graph(2)->setName("压力目标位置");
    customPlot->addGraph(); // red line
    customPlot->graph(3)->setPen(QPen(QColor(255, 40, 255)));
    customPlot->graph(3)->setName("压力传感器1");
    customPlot->addGraph(); // red line
    customPlot->graph(4)->setPen(QPen(QColor(255, 255, 40)));
    customPlot->graph(4)->setName("压力传感器2");

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    customPlot->xAxis->setTicker(timeTicker);
    customPlot->axisRect()->setupFullAxesBox();
    customPlot->yAxis->setRange(0, 10000);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));

    // setup legend:
    customPlot->legend->setFont(QFont(font().family(), 6));
    customPlot->legend->setIconSize(50, 15);
    customPlot->legend->setVisible(true);
    customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignLeft | Qt::AlignTop);

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    dataTimer.start(10); // Interval 0 means to refresh as fast as possible
    connect(&labelTimer, SIGNAL(timeout()), this, SLOT(updateLabel()));
    labelTimer.start(50);
}

void MainWindow::realtimeDataSlot()
{
    bool check;
    uint8_t type;
    uint16_t value;
    frame frame_tmp;
    data_mut.lock();
    memcpy(&frame_tmp, &mFrame, sizeof(frame));
    data_mut.unlock();
    check = checkData(frame_tmp);
    type = frame_tmp.type;
    value = (frame_tmp.value[0]<<8)|(frame_tmp.value[1]);
    if(check && (type>=cmd::T_POSITION) && (type<=cmd::PRESSURE_2)){
        mValues[type-1] = value;
    }
    static QTime timeStart = QTime::currentTime();
    // calculate two new data points:
    double key = timeStart.msecsTo(QTime::currentTime())/1000.0; // time elapsed since start of demo, in seconds
    static double lastPointKey = 0;
    if (key-lastPointKey > 0.010) // at most add point every 2 ms
    {
        // add data to lines:
        if(ui->check_target_position->isChecked()){
//            if(type == cmd::T_POSITION)
                ui->customplot->graph(0)->addData(key, mValues[0]);
        }
        if(ui->check_real_position->isChecked()){
//            if(type == cmd::R_POSITION)
                ui->customplot->graph(1)->addData(key, mValues[1]);
        }
        if(ui->check_target_pressure->isChecked()){
//            if(type == cmd::T_PRESSURE)
                ui->customplot->graph(2)->addData(key, mValues[2]);
        }
        if(ui->check_pressure1->isChecked()){
//            if(type == cmd::PRESSURE_1)
                ui->customplot->graph(3)->addData(key, mValues[3]);
        }
        if(ui->check_pressure2->isChecked()){
//            if(type == cmd::PRESSURE_2)
                ui->customplot->graph(4)->addData(key, mValues[4]);
        }

        // rescale value (vertical) axis to fit the current data:
        //ui->customPlot->graph(0)->rescaleValueAxis();
        //ui->customPlot->graph(1)->rescaleValueAxis(true);
        lastPointKey = key;
    }
    // make key axis range scroll with the data (at a constant range size of 8):
    ui->customplot->xAxis->setRange(key, 8, Qt::AlignRight);
    ui->customplot->replot();
}

void MainWindow::updateLabel()
{
    bool check;
    uint8_t type, status, mode;
    uint16_t value;
    frame frame_tmp;
    data_mut.lock();
    memcpy(&frame_tmp, &mFrame, sizeof(frame));
    data_mut.unlock();
    check = checkData(frame_tmp);
    if(check){
        type = frame_tmp.type;
        status = frame_tmp.status;
        mode = frame_tmp.work_mode;
        value = (frame_tmp.value[0]<<8)|(frame_tmp.value[1]);
        switch (type) {
        case cmd::T_POSITION:
            ui->lb_target_position->setText(QString::number(value));
            break;
        case cmd::R_POSITION:
            ui->lb_real_position->setText(QString::number(value));
            break;
        case cmd::T_PRESSURE:
            ui->lb_target_pressure->setText(QString::number(value));
            break;
        case cmd::PRESSURE_1:
            ui->lb_pressure1->setText(QString::number(value));
            break;
        case cmd::PRESSURE_2:
            ui->lb_pressure2->setText(QString::number(value));
            break;
        default:
            break;
        }
        switch(status){
        case 0 :
            ui->lb_system_status->setText("调零");
            break;
        case 1 :
            ui->lb_system_status->setText("学习");
            break;
        case 2 :
            ui->lb_system_status->setText("工作");
            break;
        case 3 :
            ui->lb_system_status->setText("故障");
            break;
        case 4 :
            ui->lb_system_status->setText("空闲");
            break;
        default:
            break;
        }
        switch(mode){
        case 1 :
            ui->lb_system_status->setText("全开");
            break;
        case 2 :
            ui->lb_system_status->setText("全关");
            break;
        case 3 :
            ui->lb_system_status->setText("压力");
            break;
        case 4 :
            ui->lb_system_status->setText("位置");
            break;
        case 5 :
            ui->lb_system_status->setText("保持");
            break;
        default:
            break;
        }
    }
}

uint8_t MainWindow::checkData(uint8_t *input, int size)
{
    uint16_t sum{0};
    for(int i=0;i<size;i++){
        sum += input[i];
    }
    return (sum&0xff);
}

bool MainWindow::checkData(const frame& frame)
{
    uint8_t buf[7] = {0};
    memcpy(buf, (const uint8_t*)&frame, sizeof(frame));
    auto temp = checkData(buf+2, 4);
    return temp==buf[6];
}

void MainWindow::convert(const QByteArray& buf, int size, frame* frame)
{
    char array[10] = {0};
    for(int i=0;i<size;i++){
        array[i] = buf.at(i);
    }
    if(frame != nullptr){
        memcpy(frame, array, size);
    }
}

