#!/usr/bin/env bash
#
# This script inserts example sources and filters into the database.
#

# Check if RSS-torrent is built
if [ ! -e `which swarmtv` ]; then
  echo "Please install SwarmTv before running this script."
  exit
fi 

# Cleaning database
echo "Delete all SQL and simple filters."
  swarmtv --del-all-simple
  swarmtv --del-filter all
echo "Done."

# INSERT INTO "sources" (name, url, filter) VALUES( "swarmtvs", "http://rsstorrents.com/rss.php?cat=8", "rsstorrent"); 
echo "Inserting sources."
  swarmtv --add-source="Eztv" --url="http://www.ezrss.it/feed/" --source-parser="defaultrss" --metatype="torrent"
  swarmtv --add-source="The Piratebay" --url="http://rss.thepiratebay.org/205" --source-parser="defaultrss" --metatype="torrent"
  swarmtv --add-source="KickAssTorrents" --url="http://www.kickasstorrents.com/tv/?rss=1" --source-parser="defaultrss" --metatype="torrent"
echo "Done."

# Although SQL filters are still supported they are not the default filter method anymore.
# Plus 99% of the filters can be written using simple filters now.
#echo "Insert SQL filters."
#./swarmtv -F "DollHouse:select id, link, title, pubdate, category, season, episode from newtorrents where title REGEXP('^[Dd]ollhouse') AND size < '400000000' AND new = 'Y'" \
#  -T "SELECT title FROM downloaded WHERE link=?1 OR (season>=?2 AND episode>=?3 AND title REGEXP('^[Dd]ollhouse'))"
#./swarmtv -F "House:select id, link, title, pubdate, category, season, episode from newtorrents where title REGEXP('^[hH]ouse') AND size < '400000000' AND new = 'Y'" \
#  -T "SELECT title FROM downloaded WHERE link=?1 OR (season>=?2 AND episode>=?3 AND title REGEXP('^[hH]ouse'))"
#./swarmtv -F "Stargate Universe:select id, link, title, pubdate, category, season, episode from newtorrents where title REGEXP('^[sS]targate [Uu]niverse') AND size < '800000000' AND new = 'Y'" \
#  -T "SELECT title FROM downloaded WHERE link=?1 OR (season>=?2 AND episode>=?3 AND title REGEXP('^[sS]targate [Uu]niverse'))"
#./swarmtv -F "BBC:select id, link, title, pubdate, category, season, episode from newtorrents where title REGEXP('^BBC') AND size < '800000000' AND new = 'Y'" \
#  -T "SELECT title FROM downloaded WHERE link=?1"
#./swarmtv -F "Mythbusters:select id, link, title, pubdate, category, season, episode from newtorrents where title REGEXP('^[Mm]ythbusters') AND size < '800000000' AND new = 'Y'" \
#  -T "SELECT title FROM downloaded WHERE link=?1 OR (season>=?2 AND episode>=?3 AND title REGEXP('^[mM]ythbusters'))"
#./swarmtv -F "Dirty Jobs:select id, link, title, pubdate, category, season, episode from newtorrents where title REGEXP('^[dD]irty [jJ]obs') AND size < '400000000' AND new = 'Y'" \
#  -T "SELECT title FROM downloaded WHERE link=?1 OR (season>=?2 AND episode>=?3 AND title REGEXP('^[dD]irty [jJ]obs'))"
#./swarmtv -F "Lost:select id, link, title, pubdate, category, season, episode from newtorrents where title REGEXP('^[Ll]ost') AND size < '400000000' AND new = 'Y'" \
#  -T "SELECT title FROM downloaded WHERE link=?1 OR (season>=?2 AND episode>=?3 AND title REGEXP('^[Ll]ost'))"
#echo "Done."


echo "Insert Simple filters."
  swarmtv --add-simple='House' --nodup='newer' --title='^house' --exclude='hunters international' --max-size='700.00 MB' --min-size='200.00 MB'
  swarmtv --add-simple='Lost' --nodup='newer' --title='^lost' --max-size='700.00 MB' --min-size='200.00 MB'
  swarmtv --add-simple='BBC' --nodup='link' --title='^bbc' --exclude='QI' --max-size='1024.00 MB' --min-size='100.00 MB'
  swarmtv --add-simple='Top Gear' --nodup='newer' --title='^top gear' --max-size='1024.00 MB' --min-size='100.00 MB'
  swarmtv --add-simple='Breaking Bad' --nodup='newer' --title='^breaking bad' --max-size='700.00 MB' --min-size='100.00 MB'
  swarmtv --add-simple='Dirty jobs' --nodup='newer' --title='^dirty jobs' --max-size='700.00 MB' --min-size='100.00 MB'
  swarmtv --add-simple='Dollhouse' --nodup='newer' --title='^dollhouse' --max-size='600.00 MB' --min-size='100.00 MB'
  swarmtv --add-simple='Computer Chronicles' --nodup='link' --title='Computer Chronicles' --max-size='800.00 MB'
echo "Done."
