@ECHO OFF

REM This script inserts example sources and filters into the database.

REM NB: I don't do windows batch scripting . . 

IF NOT EXIST swarmtv.exe ECHO "Please install SwarmTv before running this script."
IF NOT EXIST swarmtv.exe PAUSE
IF NOT EXIST swarmtv.exe EXIT

FOR /F "tokens=3 delims= " %%G IN ('REG QUERY "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" /v "Personal"') DO (SET docsdir=%%G)

mkdir %docsdir%\torrents
mkdir %docsdir%\nzb

echo "Delete all SQL and simple filters."
  swarmtv.exe --del-all-simple
  swarmtv.exe --del-filter all
echo "Done."

echo "Inserting sources."
  swarmtv.exe --add-source="Eztv" --url="http://www.ezrss.it/feed/" --source-parser="defaultrss" --metatype="torrent"
  swarmtv.exe --add-source="The Piratebay" --url="http://rss.thepiratebay.org/205" --source-parser="defaultrss" --metatype="torrent"
  swarmtv.exe --add-source="KickAssTorrents" --url="http://www.kickasstorrents.com/tv/?rss=1" --source-parser="defaultrss" --metatype="torrent"
echo "Done."

echo "Insert Simple filters."
  swarmtv.exe --add-simple="House" --nodup="newer" --title="^house" --exclude="hunters international" --max-size="700.00 MB" --min-size="200.00 MB"
  swarmtv.exe --add-simple="Lost" --nodup="newer" --title="^lost" --max-size="700.00 MB" --min-size="200.00 MB"
  swarmtv.exe --add-simple="BBC" --nodup="link" --title="^bbc" --exclude="QI" --max-size="1024.00 MB" --min-size="100.00 MB"
  swarmtv.exe --add-simple="Top Gear" --nodup="newer" --title="^top gear" --max-size="1024.00 MB" --min-size="100.00 MB"
  swarmtv.exe --add-simple="Breaking Bad" --nodup="newer" --title="^breaking bad" --max-size="700.00 MB" --min-size="100.00 MB"
  swarmtv.exe --add-simple="Dirty jobs" --nodup="newer" --title="^dirty jobs" --max-size="700.00 MB" --min-size="100.00 MB"
  swarmtv.exe --add-simple="Dollhouse" --nodup="newer" --title="^dollhouse" --max-size="600.00 MB" --min-size="100.00 MB"
  swarmtv.exe --add-simple="Computer Chronicles" --nodup="link" --title="Computer Chronicles" --max-size="800.00 MB"
echo "Done."

echo "Run once"
  swarmtv.exe -rR

