#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QTime"
#include "QMessageBox"
#include "QDebug"
#include "iostream"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
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
            qDebug()<<str.size();
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
    customPlot->axisRect()->setBackground(QPixmap("./solarpanels.jpg"));
    customPlot->addGraph();
    customPlot->graph()->setLineStyle(QCPGraph::lsLine);
    QPen pen;
    pen.setColor(QColor(255, 200, 20, 200));
    pen.setStyle(Qt::DashLine);
    pen.setWidthF(2.5);
    customPlot->graph()->setPen(pen);
    customPlot->graph()->setBrush(QBrush(QColor(255,200,20,70)));
    customPlot->graph()->setScatterStyle(QCPScatterStyle(QPixmap("./sun.png")));
    // set graph name, will show up in legend next to icon:
    customPlot->graph()->setName("Data from Photovoltaic\nenergy barometer 2011");
    // set data:
    QVector<double> year, value;
    year  << 2005 << 2006 << 2007 << 2008  << 2009  << 2010 << 2011;
    value << 2.17 << 3.42 << 4.94 << 10.38 << 15.86 << 29.33 << 52.1;
    customPlot->graph()->setData(year, value);

    // set title of plot:
    customPlot->plotLayout()->insertRow(0);
    customPlot->plotLayout()->addElement(0, 0, new QCPTextElement(customPlot, "Regenerative Energies", QFont("sans", 12, QFont::Bold)));
    // axis configurations:
    customPlot->xAxis->setLabel("Year");
    customPlot->yAxis->setLabel("Installed Gigawatts of\nphotovoltaic in the European Union");
    customPlot->xAxis2->setVisible(true);
    customPlot->yAxis2->setVisible(true);
    customPlot->xAxis2->setTickLabels(false);
    customPlot->yAxis2->setTickLabels(false);
    customPlot->xAxis2->setTicks(false);
    customPlot->yAxis2->setTicks(false);
    customPlot->xAxis2->setSubTicks(false);
    customPlot->yAxis2->setSubTicks(false);
    customPlot->xAxis->setRange(2004.5, 2011.5);
    customPlot->yAxis->setRange(0, 52);
    // setup legend:
    customPlot->legend->setFont(QFont(font().family(), 7));
    customPlot->legend->setIconSize(50, 20);
    customPlot->legend->setVisible(true);
    customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignLeft | Qt::AlignTop);
}

uint8_t MainWindow::checkData(uint8_t *input, int size)
{
    uint16_t sum{0};
    for(int i=0;i<size;i++){
        sum += input[i];
    }
    return (sum&0xff);
}

