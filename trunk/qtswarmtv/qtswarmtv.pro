#-------------------------------------------------
#
# Project created by QtCreator 2010-12-10T20:12:20
#
#-------------------------------------------------

QT       += core gui dbus network xml

TARGET = qtswarmtv
TEMPLATE = app

CONFIG += qt debug

SOURCES += main.cpp\
        mainwindow.cpp \
    swarmtv.cpp \
    swarmtvstats.cpp \
    simpletablecontrol.cpp \
    simplecell.cpp \
    simpleeditdialog.cpp \
    sourcetablecontrol.cpp \
    sourceeditdialog.cpp \
    readablesize.cpp \
    settingsdialog.cpp \
    helpdialog.cpp \
    searchcontrol.cpp \
    testfilterdialog.cpp \
    newtorrentfullinfodialog.cpp \
    downloadedtablecontrol.cpp \
    swarmtvtrayicon.cpp \
    serieslistcontrol.cpp \
    serieswidget.cpp \
    thetvdb.cpp \
    taskqueue.cpp \
    getbannertask.cpp \
    getseriestask.cpp \
    singleapplication.cpp

HEADERS  += mainwindow.h \
    singleton.h \
    swarmtv.hpp \
    swarmtvstats.hpp \
    simpletablecontrol.hpp \
    simplecell.hpp \
    simpleeditdialog.hpp \
    sourcetablecontrol.hpp \
    sourceeditdialog.hpp \
    readablesize.hpp \
    settingsdialog.hpp \
    helpdialog.hpp \
    searchcontrol.hpp \
    testfilterdialog.hpp \
    newtorrentfullinfodialog.hpp \
    downloadedtablecontrol.hpp \
    swarmtvtrayicon.h \
    serieslistcontrol.hpp \
    serieswidget.hpp \
    thetvdb.hpp \
    taskqueue.hpp \
    taskinterface.hpp \
    getbannertask.hpp \
    getseriestask.hpp \
    singleapplication.h

FORMS    += mainwindow.ui \
    simpleeditdialog.ui \
    sourceeditdialog.ui \
    settingsdialog.ui \
    helpdialog.ui \
    testfilterdialog.ui \
    newtorrentfullinfodialog.ui

LIBS 	 += -lswarmtv -ltvdb

# install
target.path = /usr/local/bin/
INSTALLS += target

RESOURCES += \
    swarmtvqtresources.qrc
