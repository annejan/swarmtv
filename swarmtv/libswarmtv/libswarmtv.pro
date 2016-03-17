######################################################################
# JinXed Thu Mar 10 00:37:33 2011 Anne Jan Brouwer
######################################################################

TEMPLATE = lib 
win32{
TARGET = libswarmtv
}
unix {
TARGET = swarmtv
}
DEPENDPATH += . \
              filehandler \
              filehandler/nzb \
              filehandler/torrent \
              srcparser/defaultrss \
              srcparser/twitter
INCLUDEPATH += . \
               srcparser/defaultrss \
               srcparser/twitter \
               filehandler \
               filehandler/torrent \
               filehandler/nzb
CONFIG += dll link_pkgconfig
PKGCONFIG += sqlite3 dbus-1 glib-2.0 dbus-glib-1
LIBS += -lcurl -lpcre
unix{
LIBS += -lesmtp
}
win32{
LIBS += -lsqlite3 -lxml2 -lcurldll -liconv -lwsock32
}
target.path = /usr/local/lib/
headers.path = /usr/local/include/
INSTALLS += target headers

# Input
HEADERS += baretitle.h \
           callback.h \
           config.h \
           curlfile.h \
           database.h \
           databaseimpl.h \
           filesystem.h \
           filter.h \
           lastdownloaded.h \
           logfile.h \
           regexp.h \
           runloop.h \
           sandboxdb.h \
           setup.h \
           simplefilter.h \
           source.h \
           stats.h \
           swarmtv.h \
           testfilter.h \
           torrentdb.h \
           torrentdownload.h \
           types.h \
           filehandler/filehandler.h \
           filehandler/nzb/findnzb.h \
           filehandler/nzb/nzbparse.h \
           filehandler/torrent/findtorrent.h \
           filehandler/torrent/tbl.h \
           filehandler/torrent/torrentparse.h \
           srcparser/defaultrss/defaultrss.h \
           srcparser/defaultrss/disectdate.h \
           srcparser/defaultrss/disectdescription.h \
           srcparser/defaultrss/rsscategory.h \
           srcparser/defaultrss/rsslink.h \
           srcparser/defaultrss/rssparse.h \
           srcparser/defaultrss/rsspubdate.h \
           srcparser/defaultrss/rssseasonepisode.h \
           srcparser/defaultrss/rssseedspeers.h \
           srcparser/defaultrss/rsssize.h \
           srcparser/defaultrss/rsstitle.h \
           srcparser/twitter/parsedate.h \
           srcparser/twitter/splittext.h \
           srcparser/twitter/twitparse.h \
           srcparser/twitter/twitter.h
SOURCES += baretitle.c \
           callback.c \
           config.c \
           curlfile.c \
           database.c \
           databaseimpl.c \
           filesystem.c \
           filter.c \
           lastdownloaded.c \
           logfile.c \
           regexp.c \
           runloop.c \
           sandboxdb.c \
           setup.c \
           simplefilter.c \
           source.c \
           stats.c \
           testfilter.c \
           torrentdb.c \
           torrentdownload.c \
           filehandler/filehandler.c \
           filehandler/nzb/findnzb.c \
           filehandler/nzb/nzbparse.c \
           filehandler/torrent/findtorrent.c \
           filehandler/torrent/tbl.c \
           filehandler/torrent/torrentparse.c \
           srcparser/defaultrss/defaultrss.c \
           srcparser/defaultrss/disectdate.c \
           srcparser/defaultrss/disectdescription.c \
           srcparser/defaultrss/rsscategory.c \
           srcparser/defaultrss/rsslink.c \
           srcparser/defaultrss/rssparse.c \
           srcparser/defaultrss/rsspubdate.c \
           srcparser/defaultrss/rssseasonepisode.c \
           srcparser/defaultrss/rssseedspeers.c \
           srcparser/defaultrss/rsssize.c \
           srcparser/defaultrss/rsstitle.c \
           srcparser/twitter/parsedate.c \
           srcparser/twitter/splittext.c \
           srcparser/twitter/twitparse.c \
           srcparser/twitter/twitter.c

win32 {
HEADERS += workarounds.h 
SOURCES += workarounds.c 
}
