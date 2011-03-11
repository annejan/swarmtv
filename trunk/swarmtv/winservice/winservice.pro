######################################################################
# JinXed Thu Mar 10 14:22:41 2011 Anne Jan Brouwer
######################################################################

TEMPLATE = app
TARGET = swarmtvservice
DEPENDPATH += . ../libswarmtv
INCLUDEPATH += . ../libswarmtv
LIBS += -L../libswarmtv -lswarmtv -lwsock32 -lws2_32
# Input
SOURCES += swarmtvservice.c
