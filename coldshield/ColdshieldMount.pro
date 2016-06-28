#-------------------------------------------------
#
# Project created by QtCreator 2016-03-08T13:57:04
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ColdshieldMount
TEMPLATE = app


SOURCES += main.cpp\
        mountcs.cpp\
		viewbuilddata.cpp\
		proteuslookup.cpp

HEADERS  += mountcs.h\
			viewbuilddata.h\
			proteuslookup.h

FORMS    += mountcs.ui\
			viewbuilddata.ui\
			proteuslookup.ui

RESOURCES += \
    res.qrc
