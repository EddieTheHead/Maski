#-------------------------------------------------
#
# Project created by QtCreator 2015-02-21T13:21:18
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Maski
TEMPLATE = app

INCLUDEPATH += "D:\\fftw"
DEPENDPATH += "D:\\fftw"

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h \

FORMS    += mainwindow.ui



win32:CONFIG(release, debug|release): LIBS += -L"D:\\fftw" -lfftw3-3
else:win32:CONFIG(debug, debug|release): LIBS += -L"D:\\fftw" -lfftw3-3
