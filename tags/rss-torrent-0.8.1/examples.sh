#!/usr/bin/env bash
#
# This script inserts example sources and filters into the database.
#

# Check if RSS-torrent is built
if [ ! -e `which rsstorrent` ]; then
  echo "Please install RSS-torrent before running this script."
  exit
fi 

# Cleaning database
echo "Delete all SQL and simple filters."
  rsstorrent --del-all-simple
  rsstorrent --del-filter all
echo "Done."

# INSERT INTO "sources" (name, url, filter) VALUES( "rsstorrents", "http://rsstorrents.com/rss.php?cat=8", "rsstorrent"); 
echo "Inserting sources."
  rsstorrent --add-source="Eztv:http://www.ezrss.it/feed/" --source-parser="defaultrss"
  rsstorrent --add-source="The Piratebay:http://rss.thepiratebay.org/205" --source-parser="defaultrss"
  rsstorrent --add-source="KickAssTorrents:http://www.kickasstorrents.com/tv/?rss=1" --source-parser="defaultrss"
echo "Done."

# Although SQL filters are still supported they are not the default filter method anymore.
# Plus 99% of the filters can be written using simple filters now.
#echo "Insert SQL filters."
#./rsstorrent -F "DollHouse:select link, title, pubdate, category, season, episode from newtorrents where title REGEXP('^[Dd]ollhouse') AND size < '400000000' AND new = 'Y'" \
#  -T "SELECT title FROM downloaded WHERE link=?1 OR (season>=?2 AND episode>=?3 AND title REGEXP('^[Dd]ollhouse'))"
#./rsstorrent -F "House:select link, title, pubdate, category, season, episode from newtorrents where title REGEXP('^[hH]ouse') AND size < '400000000' AND new = 'Y'" \
#  -T "SELECT title FROM downloaded WHERE link=?1 OR (season>=?2 AND episode>=?3 AND title REGEXP('^[hH]ouse'))"
#./rsstorrent -F "Stargate Universe:select link, title, pubdate, category, season, episode from newtorrents where title REGEXP('^[sS]targate [Uu]niverse') AND size < '800000000' AND new = 'Y'" \
#  -T "SELECT title FROM downloaded WHERE link=?1 OR (season>=?2 AND episode>=?3 AND title REGEXP('^[sS]targate [Uu]niverse'))"
#./rsstorrent -F "BBC:select link, title, pubdate, category, season, episode from newtorrents where title REGEXP('^BBC') AND size < '800000000' AND new = 'Y'" \
#  -T "SELECT title FROM downloaded WHERE link=?1"
#./rsstorrent -F "Mythbusters:select link, title, pubdate, category, season, episode from newtorrents where title REGEXP('^[Mm]ythbusters') AND size < '800000000' AND new = 'Y'" \
#  -T "SELECT title FROM downloaded WHERE link=?1 OR (season>=?2 AND episode>=?3 AND title REGEXP('^[mM]ythbusters'))"
#./rsstorrent -F "Dirty Jobs:select link, title, pubdate, category, season, episode from newtorrents where title REGEXP('^[dD]irty [jJ]obs') AND size < '400000000' AND new = 'Y'" \
#  -T "SELECT title FROM downloaded WHERE link=?1 OR (season>=?2 AND episode>=?3 AND title REGEXP('^[dD]irty [jJ]obs'))"
#./rsstorrent -F "Lost:select link, title, pubdate, category, season, episode from newtorrents where title REGEXP('^[Ll]ost') AND size < '400000000' AND new = 'Y'" \
#  -T "SELECT title FROM downloaded WHERE link=?1 OR (season>=?2 AND episode>=?3 AND title REGEXP('^[Ll]ost'))"
#echo "Done."


echo "Insert Simple filters."
  rsstorrent --add-simple='House' --nodup='newer' --title='^house' --exclude='hunters international' --max-size='700.00 MB' --min-size='200.00 MB'
  rsstorrent --add-simple='Lost' --nodup='newer' --title='^lost' --max-size='700.00 MB' --min-size='200.00 MB'
  rsstorrent --add-simple='BBC' --nodup='link' --title='^bbc' --exclude='QI' --max-size='1024.00 MB' --min-size='100.00 MB'
  rsstorrent --add-simple='Top Gear' --nodup='newer' --title='^top gear' --max-size='1024.00 MB' --min-size='100.00 MB'
  rsstorrent --add-simple='Breaking Bad' --nodup='newer' --title='^breaking bad' --max-size='700.00 MB' --min-size='100.00 MB'
  rsstorrent --add-simple='Dirty jobs' --nodup='newer' --title='^dirty jobs' --max-size='700.00 MB' --min-size='100.00 MB'
  rsstorrent --add-simple='Dollhouse' --nodup='newer' --title='^dollhouse' --max-size='600.00 MB' --min-size='100.00 MB'
  rsstorrent --add-simple='Computer Chronicles' --nodup='link' --title='Computer Chronicles' --max-size='800.00 MB'
echo "Done."
