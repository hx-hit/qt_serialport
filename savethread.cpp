#include "savethread.h"
#include <QDebug>
#include <thread>
SaveThread::SaveThread(QString name, std::mutex *mut, std::deque<uint16_t> *data_que, bool* flag)
{
    mMut = mut;
    mDataQueue = data_que;
    saveFlag = flag;
    mSaveName = name;
}

void SaveThread::run()
{
    QXlsx::Document xlsx;
    int cnt=1;
    while(*saveFlag){
        if(!mDataQueue->empty())
        {
            mMut->lock();
            auto data = mDataQueue->front();
            mDataQueue->pop_front();
            mMut->unlock();
            if(mSaveName == "pressureSensor1" || mSaveName == "pressureSensor2")
                xlsx.write(cnt++, 1, data*10);
            else
                xlsx.write(cnt++, 1, data);
            std::cout<<"get data : "<<data<<std::endl;
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
        qDebug()<<__LINE__;
    }
    /*保存文件*/
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy-MM-dd-hh-mm");
    xlsx.saveAs(mSaveName + "_" + current_date + ".xlsx");
    qDebug()<<"save "<<mSaveName + "_" + current_date + ".xlsx";
    if(!mDataQueue->empty()){
        mDataQueue->clear();
    }
}
