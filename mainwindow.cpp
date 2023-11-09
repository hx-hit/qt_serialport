#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QTime"
#include "QMessageBox"
#include "QDebug"
#include "cmd.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    for(int i=0;i<5;i++){
        mValues[i] = 0;
    }
    stop_display = false;
    ui->setupUi(this);
    mPool = QThreadPool::globalInstance();
    mPool->setMaxThreadCount(8);
   for(int i=0;i<5;i++){
        std::deque<uint16_t> temp;
        saveDataMap.insert(std::pair<uint8_t, std::deque<uint16_t>>(i, temp));
    }
    serialport = new QSerialPort;
    GetAveriablePort();
    PortConfigureInit();
    setupQuadraticDemo(ui->customplot);
    ui->customplot->replot();
    saveAllThread = new std::thread(&MainWindow::save_thread_function, this);
    saveAllThread->detach();
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
            auto type = mFrame.type;
            auto index = type - 1;
            if(saveFlagVector[index]){
                uint16_t value = (mFrame.value[0]<<8)|(mFrame.value[1]);
                auto& temp_deque = saveDataMap[index];
                queue_mut.lock();
                temp_deque.push_back(value);
                queue_mut.unlock();
                std::cout<<"******* :: "<<temp_deque.size()<<std::endl;
            }
        }
        if(textstate_receive == true)   //文本模式
        {
            QString str = ui->recv_text_window->toPlainText();
            if(str.size()>240){
                str = str.right(240);
            }
            str += tr(buf);
            str += " ";
            ui->recv_text_window->clear();
            ui->recv_text_window->append(str);
        }
        if(textstate_receive == false)   //非文本模式
        {
            QString str = ui->recv_text_window->toPlainText();
            if(str.size()>240){
                str = str.right(240);
            }
            // byteArray 转 16进制
            QByteArray temp = buf.toHex();
            str += tr(temp);
            str += "  ";
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
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
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
    timeTicker->setTickCount(10);
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
    dataTimer.start(25); // Interval 0 means to refresh as fast as possible
#ifndef USE_THREAD
    connect(&labelTimer, SIGNAL(timeout()), this, SLOT(updateLabel()));
    labelTimer.start(20);
#else
    showLabel = new std::thread(&MainWindow::show_label_function, this);
#endif
}

void MainWindow::save_thread_function()
{
    QXlsx::Document xlsx;
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy-MM-dd-hh-mm");
    long count0{1}, count1{1}, count2{1}, count3{1}, count4{1};
    std::deque<uint16_t> *data_que_0, *data_que_1, *data_que_2, *data_que_3, *data_que_4;
    while(1){
        std::unique_lock<std::mutex>lk(cond_mut);
        queue_cond.wait(lk, [&](){return save_flag;});
        m_xlsx = new Document(m_saveName, this);
        if(m_xlsx->load()){
            qDebug() << "excel打开成功!";
            m_xlsx->write(count0++, 1, "target_position");
            m_xlsx->write(count1++, 2, "real_position");
            m_xlsx->write(count2++, 3, "target_pressure");
            m_xlsx->write(count3++, 4, "sensor1");
            m_xlsx->write(count4++, 5, "sensor2");
        }else{
            qDebug() << "excel打开失败!";
            continue;
        }
        data_que_0 = &saveDataMap[0];
        data_que_1 = &saveDataMap[1];
        data_que_2 = &saveDataMap[2];
        data_que_3 = &saveDataMap[3];
        data_que_4 = &saveDataMap[4];
        while(save_all)
        {
#if 1
            if(!data_que_0->empty())
            {
                queue_mut.lock();
                auto data0 = data_que_0->front();
                data_que_0->pop_front();
                queue_mut.unlock();
                m_xlsx->write(count0++, 1, data0);
            }
            if(!data_que_1->empty())
            {
                queue_mut.lock();
                auto data1 = data_que_1->front();
                data_que_1->pop_front();
                queue_mut.unlock();
                m_xlsx->write(count1++, 2, data1);
            }
            if(!data_que_2->empty())
            {
                queue_mut.lock();
                auto data2 = data_que_2->front();
                data_que_2->pop_front();
                queue_mut.unlock();
                m_xlsx->write(count2++, 3, data2);
            }
            if(!data_que_3->empty())
            {
                queue_mut.lock();
                auto data3 = data_que_3->front();
                data_que_3->pop_front();
                queue_mut.unlock();
                m_xlsx->write(count3++, 4, data3*10);
            }
            if(!data_que_4->empty())
            {
                queue_mut.lock();
                auto data4 = data_que_4->front();
                data_que_4->pop_front();
                queue_mut.unlock();
                m_xlsx->write(count4++, 5, data4*10);
            }
#else
            m_xlsx->write(count1++, 1, 1);
            m_xlsx->write(count3++, 2, 2);
            m_xlsx->write(count4++, 3, 3);
#endif
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

        }
        if(m_xlsx->save())
        {
            qDebug() << "数据写入成功！";
            queue_mut.lock();
            data_que_0->clear();
            data_que_1->clear();
            data_que_2->clear();
            data_que_3->clear();
            data_que_4->clear();
            queue_mut.unlock();
            count0 = 1;
            count1 = 1;
            count2 = 1;
            count3 = 1;
            count4 = 1;
        }
        else
        {
            qDebug() << "数据写入失败！";
        }
    }

}

void MainWindow::show_label_function()
{
    while(1){
        updateLabel();
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
}

void MainWindow::draw_graphy_function()
{
    while(1){
//        realtimeDataSlot();
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
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
    if(stop_display){
        return;
    }
    // calculate two new data points:
    double key = timeStart.msecsTo(QTime::currentTime())/1000.0; // time elapsed since start of demo, in seconds
    static double lastPointKey = 0;
    if (key-lastPointKey > 0.005) // at most add point every 2 ms
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
    if(key > 600){
        for(int i=0;i<5;i++){
                ui->customplot->graph(i)->removeDataBefore(key-600.0);
        }
    }
    // make key axis range scroll with the data (at a constant range size of 8):
    ui->customplot->xAxis->setRange(key, 600, Qt::AlignRight);
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
            ui->lb_pressure1->setText(QString::number(value*10));
            break;
        case cmd::PRESSURE_2:
            ui->lb_pressure2->setText(QString::number(value*10));
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
            ui->lb_valve_mode->setText("全开");
            break;
        case 2 :
            ui->lb_valve_mode->setText("全关");
            break;
        case 3 :
            ui->lb_valve_mode->setText("压力");
            break;
        case 4 :
            ui->lb_valve_mode->setText("位置");
            break;
        case 5 :
            ui->lb_valve_mode->setText("保持");
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
    if(frame != nullptr){
        memcpy(frame, buf.data(), size);
    }
}


void MainWindow::on_btn_stop_clicked()
{
    if(ui->btn_stop->text() == "暂停"){
        stop_display = true;
        ui->btn_stop->setText("运行");
    }
    else{
        ui->btn_stop->setText("暂停");
        stop_display = false;
    }
}

void MainWindow::on_btn_save_t_po_clicked()
{

    if(ui->btn_save_t_po->text() == "保存"){
        ui->btn_save_t_po->setText("停止");
        saveFlagVector[0] = true;
        save_t_po = true;
        auto& data_que = saveDataMap[0];
        mSaveThread[0] = new SaveThread("targetPosition", &queue_mut, &data_que, &save_t_po);
        mSaveThread[0]->setAutoDelete(true);
        mPool->start(mSaveThread[0]);
    }else{
        ui->btn_save_t_po->setText("保存");
        save_t_po = false;
        saveFlagVector[0] = false;
    }
}

void MainWindow::on_btn_save_r_po_clicked()
{
    if(ui->btn_save_r_po->text() == "保存"){
        ui->btn_save_r_po->setText("停止");
        saveFlagVector[1] = true;
        save_r_po = true;
        auto& data_que = saveDataMap[1];
        mSaveThread[1] = new SaveThread("realPosition", &queue_mut, &data_que, &save_r_po);
        mSaveThread[1]->setAutoDelete(true);
        mPool->start(mSaveThread[1]);
    }else{
        ui->btn_save_r_po->setText("保存");
        save_r_po = false;
        saveFlagVector[1] = false;
    }
}


void MainWindow::on_btn_save_t_pr_clicked()
{
    if(ui->btn_save_t_pr->text() == "保存"){
        ui->btn_save_t_pr->setText("停止");
        save_t_pr = true;
        saveFlagVector[2] = true;
        auto& data_que = saveDataMap[2];
        mSaveThread[2] = new SaveThread("targetPressure", &queue_mut, &data_que, &save_t_pr);
        mSaveThread[2]->setAutoDelete(true);
        mPool->start(mSaveThread[2]);
    }else{
        ui->btn_save_t_pr->setText("保存");
        save_t_pr = false;
        saveFlagVector[2] = false;
    }
}

void MainWindow::on_btn_save_s1_clicked()
{
    if(ui->btn_save_s1->text() == "保存"){
        ui->btn_save_s1->setText("停止");
        save_r_s1 = true;
        saveFlagVector[3] = true;
        auto& data_que = saveDataMap[3];
        mSaveThread[3] = new SaveThread("pressureSensor1", &queue_mut, &data_que, &save_r_s1);
        mSaveThread[3]->setAutoDelete(true);
        mPool->start(mSaveThread[3]);
    }else{
        ui->btn_save_s1->setText("保存");
        save_r_s1 = false;
        saveFlagVector[3] = false;
    }
}

void MainWindow::on_btn_save_s2_clicked()
{
    if(ui->btn_save_s2->text() == "保存"){
        ui->btn_save_s2->setText("停止");
        save_r_s2 = true;
        saveFlagVector[4] = true;
        auto& data_que = saveDataMap[4];
        mSaveThread[4] = new SaveThread("pressureSensor2", &queue_mut, &data_que, &save_r_s2);
        mSaveThread[4]->setAutoDelete(true);
        mPool->start(mSaveThread[4]);
    }else{
        ui->btn_save_s2->setText("保存");
        save_r_s2 = false;
        saveFlagVector[4] = false;
    }
}



void MainWindow::on_btn_save_all_clicked()
{
    QString file_name = "all_";
    if(ui->btn_save_all->text() == "保存所有"){
        ui->btn_save_all->setText("停止");
        save_all = true;
        save_flag = true;
        saveFlagVector[0] = true;
        saveFlagVector[1] = true;
        saveFlagVector[2] = true;
        saveFlagVector[3] = true;
        saveFlagVector[4] = true;
        QDateTime current_date_time = QDateTime::currentDateTime();
        QString current_date =current_date_time.toString("yyyy-MM-dd-hh-mm");
        m_saveName = file_name + current_date + ".xlsx";
        QXlsx::Document xlsx;                      // 初始化后默认有一个sheet1
        bool ret = xlsx.saveAs(m_saveName); // 保存到EXCEL_NAME，如果已经存在则覆盖
        if(ret)
        {
            qDebug() << "创建excel成功！";
            queue_cond.notify_one();
            ++file_count;
        }
        else
        {
            qDebug() << "创建excel失败！";
        }

    }else{
        ui->btn_save_all->setText("保存所有");
        save_all = false;
        save_flag = false;
        saveFlagVector[0] = false;
        saveFlagVector[1] = false;
        saveFlagVector[2] = false;
        saveFlagVector[3] = false;
        saveFlagVector[4] = false;
    }
}

void MainWindow::on_actionFlowRate_triggered()
{
    if(mFlowRate == nullptr){
        mFlowRate = new FlowRate();
        mFlowRate->show();
    }

}

