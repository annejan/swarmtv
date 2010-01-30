#!/bin/bash
# This script inserts example sources and filters into the database.
#

# Check if rsstorrent is built
if [ ! -e ./rsstorrent ]; then
  echo "Please build rsstorrent first before running this script."
  exit
fi 

# Cleaning database
echo "Delete all filters."
./rsstorrent -d all
echo "Done."

# INSERT INTO "sources" (name, url, filter) VALUES( "rsstorrents", "http://rsstorrents.com/rss.php?cat=8", "rsstorrent"); 
echo "Inserting sources."
./rsstorrent -s "Eztv:http://www.ezrss.it/feed/" -t "defaultrss"
./rsstorrent -s "The Piratebay:http://rss.thepiratebay.org/205" -t "defaultrss"
echo "Done."

echo "Insert SQL filters."
./rsstorrent -F "DollHouse:select link, title, pubdate, category, season, episode from newtorrents where title REGEXP('^[Dd]ollhouse') AND size < '400000000' AND new = 'Y'" \
  -T "SELECT title FROM downloaded WHERE link=?1 OR (season>=?2 AND episode>=?3 AND title REGEXP('^[Dd]ollhouse'))"
./rsstorrent -F "House:select link, title, pubdate, category, season, episode from newtorrents where title REGEXP('^[hH]ouse') AND size < '400000000' AND new = 'Y'" \
  -T "SELECT title FROM downloaded WHERE link=?1 OR (season>=?2 AND episode>=?3 AND title REGEXP('^[hH]ouse'))"
./rsstorrent -F "Stargate Universe:select link, title, pubdate, category, season, episode from newtorrents where title REGEXP('^[sS]targate [Uu]niverse') AND size < '800000000' AND new = 'Y'" \
  -T "SELECT title FROM downloaded WHERE link=?1 OR (season>=?2 AND episode>=?3 AND title REGEXP('^[sS]targate [Uu]niverse'))"
./rsstorrent -F "BBC:select link, title, pubdate, category, season, episode from newtorrents where title REGEXP('^BBC') AND size < '800000000' AND new = 'Y'" \
  -T "SELECT title FROM downloaded WHERE link=?1"
./rsstorrent -F "Mythbusters:select link, title, pubdate, category, season, episode from newtorrents where title REGEXP('^[Mm]ythbusters') AND size < '800000000' AND new = 'Y'" \
  -T "SELECT title FROM downloaded WHERE link=?1 OR (season>=?2 AND episode>=?3 AND title REGEXP('^[mM]ythbusters'))"
./rsstorrent -F "Dirty Jobs:select link, title, pubdate, category, season, episode from newtorrents where title REGEXP('^[dD]irty [jJ]obs') AND size < '400000000' AND new = 'Y'" \
  -T "SELECT title FROM downloaded WHERE link=?1 OR (season>=?2 AND episode>=?3 AND title REGEXP('^[dD]irty [jJ]obs'))"
./rsstorrent -F "Lost:select link, title, pubdate, category, season, episode from newtorrents where title REGEXP('^[Ll]ost') AND size < '400000000' AND new = 'Y'" \
  -T "SELECT title FROM downloaded WHERE link=?1 OR (season>=?2 AND episode>=?3 AND title REGEXP('^[Ll]ost'))"
echo "Done."


echo "Insert Simple filters."
 ./rsstorrent --add-simple House --title "^[Hh]ouse" --minsize="200mb" --maxsize="700mb" --nodup="newer"
echo "Done."
