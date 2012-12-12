#-------------------------------------------------
#
# Project created by QtCreator 2012-12-11T18:13:57
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MultiscreenManga
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

HEADERS += \
    renderwidget.h

SOURCES += \
    renderwidget.cpp

HEADERS += \
    mangavolume.h

SOURCES += \
    mangavolume.cpp
