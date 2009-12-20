#! /usr/bin/env bash
DBFILE="./rss.db"

# When a database exists, delete and recreate
if [ -e $DBFILE ]
then
  echo "Found old db '$DBFILE' file, removing it."
  rm $DBFILE
fi


# creating a new database file

echo "Creating newtorrents table."
sqlite3 ./rss.db <<SQL_ENTRY_TAG_1
create table newtorrents (id INTEGER PRIMARY KEY,title TEXT, link TEXT UNIQUE, pubdate DATE,
category TEXT, season INTEGER, episode INTEGER, seeds INTEGER DEFAULT 0, peers INTEGER DEFAULT 0, size INTEGER, new TEXT DEFAULT 'Y');
SQL_ENTRY_TAG_1
echo "Done."

echo "Creating downloaded table."
sqlite3 ./rss.db <<SQL_ENTRY_TAG_2
create table downloaded (id INTEGER PRIMARY KEY,title TEXT, link TEXT UNIQUE, pubdate DATE,
category TEXT, season INTEGER, episode INTEGER, date DATE);
SQL_ENTRY_TAG_2
echo "Done."

echo "Creating filters table."
sqlite3 ./rss.db <<SQL_ENTRY_TAG_3
create table filters (id INTEGER PRIMARY KEY, name TEXT UNIQUE, filter TEXT, nodouble TEXT DEFAULT "");
SQL_ENTRY_TAG_3
echo "done."

echo "Creating config table."
sqlite3 ./rss.db <<SQL_ENTRY_TAG_4
create table config (id INTEGER PRIMARY KEY, prop TEXT UNIQUE, value TEXT, descr TEXT);
INSERT INTO "config" (prop, value, descr) VALUES("torrentdir", "./torrents", "Path the downloaded torrents are placed in.");
INSERT INTO "config" (prop, value, descr) VALUES("logfile", "./logfile.txt", "Path to logfile.");
INSERT INTO "config" (prop, value, descr) VALUES("lockfile", "./lockfile.pid", "Path to lockfile.");
INSERT INTO "config" (prop, value, descr) VALUES("refresh", "3600", "Seconds between refreshes.");
INSERT INTO "config" (prop, value, descr) VALUES("default_filter", "eztv", "The default rss filter to add to new rss sources.");
INSERT INTO "config" (prop, value, descr) VALUES("smtp_enable", "N", "'Y' is send email notifications on new download, 'N' is don't.");
INSERT INTO "config" (prop, value, descr) VALUES("smtp_to", "foo@bar.nl", "Host to send the notifications to.");
INSERT INTO "config" (prop, value, descr) VALUES("smtp_from", "user@somehost.nl", "The from email-address in the mail headers.");
INSERT INTO "config" (prop, value, descr) VALUES("smtp_host", "smtp.foobar.nl:25", "The STMP server used to send the notifications.");
SQL_ENTRY_TAG_4
echo "Done."

echo "Creating urls table."
sqlite3 ./rss.db <<SQL_ENTRY_TAG_5
create table sources (id INTEGER PRIMARY KEY, name TEXT UNIQUE, url TEXT, filter TEXT);
SQL_ENTRY_TAG_5
echo "Done."

echo "showing table contents."
sqlite3 ./rss.db <<SQL_ENTRY_TAG_6
select * from sqlite_master;
SQL_ENTRY_TAG_6

