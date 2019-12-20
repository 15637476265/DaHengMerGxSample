#-------------------------------------------------
#
# Project created by QtCreator 2019-11-13T09:47:05
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CameraTest
TEMPLATE = app

dsdssdsdsds
#环境变量设置为GxAPI的安装目录
DAHENG_ROOT = /SDK/Galaxy_Linux_x86_U2_CN1_0/Galaxy_U2
GENICAM_ROOT_V2_3 =/SDK/Galaxy_Linux_x86_U2_CN1_0/Galaxy_U2/sdk/genicam
OPENCV_HOME=/SDK/opencv320
BOOST_HOME = /SDK/boost1640


INCLUDEPATH += $${PROROOT}/include \
               $${BOOST_HOME}/include \
               $${OPENCV_HOME}/include \
               $${DAHENG_ROOT}/sdk/include \
               $${GENICAM_ROOT_V2_3}/library/CPP/include \

LIBS += -L$${GENICAM_ROOT_V2_3}/bin/Linux64_x64/GenApi/Generic \
            -lGCBase_gcc40_v2_3 -lGenApi_gcc40_v2_3 -llog4cpp_gcc40_v2_3 -lLog_gcc40_v2_3 -lMathParser_gcc40_v2_3 \
        -L$${OPENCV_HOME}/lib \
            -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_core -lopencv_tracking -lopencv_video -lopencv_dnn -lopencv_tracking \
        -L$${PROROOT}/lib \
        -L$${BOOST_HOME}/lib/release \
        -L$${DAHENG_ROOT}/sdk/lib \
        -L$${DAHENG_ROOT}/sdk/lib/pub \
            -ldhgentl -lgxiapi -ldximageproc   \ #-ldximageproc
        -L$${GENICAM_ROOT_V2_3}/bin/Linux64_x64 \
        -lboost_filesystem -lboost_system -lboost_date_time

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        src/Method/CameraHelper.cpp \
    src/Method/DataRecvThread.cpp \
    src/Widget/QCvDisplay.cpp

HEADERS += \
        mainwindow.h \
        src/Method/CameraHelper.h \
    src/Method/DataRecvThread.h \
    src/Widget/QCvDisplay.h

FORMS += \
        mainwindow.ui


DEFINES += QT_DEPRECATED_WARNINGS
