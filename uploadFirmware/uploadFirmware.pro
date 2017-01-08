#-------------------------------------------------
#
# Project created by QtCreator 2016-11-17T12:36:22
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network

TARGET = uploadFirmware
TEMPLATE = app

ICON = uploadIcon.icns

SOURCES += main.cpp\
        uploadfirmware.cpp \
    about.cpp \
    targetfinder.cpp

HEADERS  += uploadfirmware.h \
    about.h \
    targetfinder.h

FORMS    += uploadfirmware.ui \
    about.ui \
    targetfinder.ui
