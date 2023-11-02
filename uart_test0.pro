QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++17
QMAKE_CXXFLAGS += -Wa,-mbig-obj
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QXLSX_PARENTPATH=./QXlsx/         # current QXlsx path is . (. means curret directory)
QXLSX_HEADERPATH=./QXlsx/header/  # current QXlsx header path is ./header/
QXLSX_SOURCEPATH=./QXlsx/source/  # current QXlsx source path is ./source/
include(./QXlsx/QXlsx.pri)

DEFINES += USE_THREAD

SOURCES += \
    customplot/qcustomplot.cpp \
    main.cpp \
    mainwindow.cpp \
    savethread.cpp

HEADERS += \
    RingBuffer.hpp \
    cmd.h \
    customplot/qcustomplot.h \
    mainwindow.h \
    savethread.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
