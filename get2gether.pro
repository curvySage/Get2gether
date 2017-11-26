#-------------------------------------------------
#
# Project created by QtCreator 2017-09-08T00:58:11
#
#-------------------------------------------------

QT += core
QT += gui
QT += sql

CONFIG += console

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = get2gether
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    dashboard.cpp \
    connection.cpp \
    dialog.cpp \
    grouppopup.cpp \
    mythread.cpp \
    edit.cpp \
    display.cpp \
    delete.cpp \
    paintCell.cpp \

HEADERS += \
        mainwindow.h \
    dashboard.h \
    connection.h \
    dialog.h \
    grouppopup.h \
    mythread.h \
    edit.h \
    display.h \
    delete.h \
    paintCell.h \

FORMS += \
        mainwindow.ui \
    dashboard.ui \
    dialog.ui \
    grouppopup.ui

RC_ICONS = get2gether_icon.ico

RESOURCES += \
    assets.qrc

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/connector/mac/lib/release/ -lmysqlclient.18
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/connector/mac/lib/debug/ -lmysqlclient.18
else:unix: LIBS += -L$$PWD/connector/mac/lib/ -lmysqlclient.18

INCLUDEPATH += $$PWD/connector/mac/include
DEPENDPATH += $$PWD/connector/mac/include
