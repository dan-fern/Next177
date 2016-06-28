#-------------------------------------------------
#
# Project created by QtCreator 2016-03-12T13:34:45
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ColdfilterMount
TEMPLATE = app


SOURCES += main.cpp\
        mountcf.cpp\
		viewbuilddata.cpp\
		proteuslookup.cpp

HEADERS  += mountcf.h\
		viewbuilddata.h\
		proteuslookup.h

FORMS    += mountcf.ui\
		viewbuilddata.ui\
		proteuslookup.ui

RESOURCES += \
    res.qrc
