#ifndef SAVETHREAD_H
#define SAVETHREAD_H

#include <QThreadPool>
#include <deque>
#include <iostream>
#include <mutex>
#include "xlsxdocument.h"
#include "xlsxchartsheet.h"
#include "xlsxcellrange.h"
#include "xlsxchart.h"
#include "xlsxrichstring.h"
#include "xlsxworkbook.h"
#include <QDateTime>
using namespace QXlsx;

class SaveThread : public QRunnable
{
public:
    SaveThread(QString name, std::mutex *mut, std::deque<uint16_t>* data_que, bool* flag);
private:
    void run() override;
private:
    std::mutex *mMut;
    std::deque<uint16_t>* mDataQueue;
    int mIndex;
    bool* saveFlag;
    QString mSaveName;
};

#endif // SAVETHREAD_H
