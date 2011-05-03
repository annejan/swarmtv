######################################################################
# JinXed Thu Mar 10 00:40:49 2011 Anne Jan Brouwer
######################################################################

TEMPLATE = app
TARGET = swarmtv
DEPENDPATH += .
INCLUDEPATH += . ../libswarmtv
CONFIG += link_pkgconfig
PKGCONFIG += xml2po dbus-1 glib-2.0 dbus-glib-1 sqlite3
LIBS += -L../libswarmtv -lswarmtv -lcurl -lpcre
unix{
 LIBS += -lesmtp -ldbus-1 -lglib-2.0 -ldbus-glib-1
}
win32{
LIBS += -lxml2 -lwsock32 -lws2_32 -liconv
}
target.path = /usr/local/bin/
INSTALLS += target
# Input
HEADERS += daemonize.h \
           dbus.h \
           frontfuncts.h \
           handleopts.h \
           mailmsg.h \
           present.h \
           runloop.h \
           simplewizard.h \
           xmlencode.h
SOURCES += daemonize.c \
           frontfuncts.c \
           handleopts.c \
           present.c \
           runloop.c \
           simplewizard.c \
           swarmtv.c \
           xmlencode.c
unix{
SOURCES += mailmsg.c \
           dbus.c
}
