#-------------------------------------------------
#
# Project created by QtCreator 2010-12-10T20:12:20
#
#-------------------------------------------------

QT       += core gui

TARGET = qtswarmtv
TEMPLATE = app


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
    downloadedtablecontrol.cpp

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
    downloadedtablecontrol.hpp

FORMS    += mainwindow.ui \
    simpleeditdialog.ui \
    sourceeditdialog.ui \
    settingsdialog.ui \
    helpdialog.ui \
    testfilterdialog.ui \
    newtorrentfullinfodialog.ui

LIBS 	 += -lswarmtv

# install
target.path = /usr/local/
INSTALLS += target

