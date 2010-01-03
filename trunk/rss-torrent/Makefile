CFLAGS=-g -c -Wall -I /usr/include/libxml2 -I. 
LDFLAGS=-lsqlite3 -lpcre -lcurl -lxml2 -lesmtp -lmagic

all: rsstorrent
	
rsstorrent: rsstorrent.o database.o config.o regexp.o source.o filter.o runloop.o curlfile.o rssfilter/rsstorrent/rsstorrentfilter.o rssfilter/eztv/eztvfilter.o torrentdb.o torrentdownload.o logfile.o daemonize.o mailmsg.o findtorrent.o mymagic.o sandboxdb.o testfilter.o

clean:
	@rm *.o rsstorrent rssfilter/*/*.o

test:
	./rsstorrent -h

db:
	if [ -e "./rss.db" ] ; then rm ./rss.db ; fi
	./createdb.sh

examples:
	./examples.sh

distclean: clean
	if [ -e "./rss.db" ] ; then rm rss.db ; fi
	if [ -e "./logfile.txt" ] ; then rm logfile.txt ; fi
	if [ -e "./lockfile.pid" ] ; then rm lockfile.pid ; fi
