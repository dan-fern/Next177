#-------------------------------------------------
#
# Project created by QtCreator 2016-03-09T09:48:49
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MotherboardMount
TEMPLATE = app


SOURCES += main.cpp\
        mountmb.cpp\
		viewbuilddata.cpp

HEADERS  += mountmb.h\
		viewbuilddata.h

FORMS    += mountmb.ui\
		viewbuilddata.ui

RESOURCES += \
    res.qrc
