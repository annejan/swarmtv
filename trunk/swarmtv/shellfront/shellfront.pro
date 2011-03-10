######################################################################
# JinXed Thu Mar 10 00:40:49 2011 Anne Jan Brouwer
######################################################################

TEMPLATE = app
TARGET = swarmtv
DEPENDPATH += .
INCLUDEPATH += . ../libswarmtv
CONFIG += link_pkgconfig
PKGCONFIG += xml2po dbus-1 glib-2.0 dbus-glib-1 sqlite3
LIBS += -L../libswarmtv -lswarmtv -lcurl 
unix{
LIBS += -lesmtp
}
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
           dbus.c \
           frontfuncts.c \
           handleopts.c \
           mailmsg.c \
           present.c \
           runloop.c \
           simplewizard.c \
           swarmtv.c \
           xmlencode.c
