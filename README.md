## Introduction

SwarmTv is a broadcatching tool with a stable commandline interface.
A QT interface is in the works at the moment, a development version can be found in the [GIT](https://github.com/annejan/swarmtv).
Swarmtv handles RSS feeds and downloads using NZB and torrents.
RSS data can be filtered using regexp driven filters that can be defined using many parameters.
Swarmtv is tested on Ubuntu Linux on both 32 and 64-bit operating systems and currently being ported to [windows](http://swarmtv.nl/windows.html).
Testing on FreeBSD worked like a charm, still need to test MacOsX!
For Ubuntu Natty Narwhal users a ppa repository is available **ppa:ranzbak/ppaswarmtv**

## Build this program

The name change of RSS-Torrent to Swarmtv also changed the program  and config settings, a guide for moving from RSS-Torrent to Swarmtv while keeping your data is included in the release documents.

Libraries :

| library | home page |
| --- | --- |
| Sqlite3 | [http://www.sqlite.org/](http://www.sqlite.org/) |
| Libpcre | [http://www.pcre.org/](http://www.pcre.org/) |
| LibCURL | [http://curl.haxx.se/libcurl/](http://curl.haxx.se/libcurl/) |
| LibXML2 | [http://xmlsoft.org/](http://xmlsoft.org/) |
| optional |
| GlibDBus | [http://www.freedesktop.org/wiki/Software/dbus](http://www.freedesktop.org/wiki/Software/dbus) |
| Libesmtp | [http://www.stafford.uklinux.net/libesmtp/](http://www.stafford.uklinux.net/libesmtp/) |

Build command:
```
$ qmake
$ make
$ make install
```
Setup Swarmtv configuration values.
```
$ swarmtv --list-config
$ swarmtv --set-config="<item>:<value>"            <- have a look at the man page for the settings and there meaning.
```
The directory to here the meta files are downloaded **must exist** and be writable to the user running Swarmtv.
Change the directory using # swarmtv -C "torrentdir:<path your torrents need to go>"
Or create the directory on the default path # mkdir ~/torrents ; mkdir ~/nzb

To create some example filters and sources in the database

<pre># make examples</pre>

For examples look at the examples.sh file

## Setting up config variables

List configuration values :
```
$ swarmtv --list-config
```
Set a config value :
```
$ swarmtv --set-config <name:value>
```
Example :
```
$ swarmtv --set-config="refresh:1800"
```
Config values :

| Value | Description |
| --- | --- |
| torrentdir | Path the downloaded torrents are placed in. (and the torrent software should monitor for torrents) |
| nzbdir | Path the downloaded nzbs are placed in. (and the torrent software should monitor for torrents) |
| logfile | Path to log file. |
| lockfile | Path to lock file. (The lock file contains the PID op the current running daemon) |
| refresh | Seconds between refreshes. (fetching RSS feeds) |
| default_filter | The defaultrss filter to add to new RSS sources. |
| smtp_enable | 'Y' is send email notifications on downloading a new torrent, 'N' is don't. |
| smtp_to | Email address  to send the notifications to. |
| smtp_from | The from email address the email shows. |
| smtp_host | The STMP server used to send the notifications. |
| retain | The number of days feed information is retained. |
| min_size | Minimal content size that is trusted, sometimes the size of the meta file is provided not the size of the content contained therin. When the size is below this threshold the meta files are downloaded to check if the size is correct. (size is in bytes) |

## Running Swarmtv

To start Swarmtv in daemon mode :

$ swarmtv --run

To start Swarmtv without detaching from the terminal :

$ swarmtv --run --nodetach

In order to see if Swarmtv is still running :

$ tail -f ~/.swarmtv/swarmtv.log

To stop Swarmtv
```
$ kill `cat ~/.swarmtv/lockfile.pid`
```
## Adding sources

### Example
```
$ swarmtv --add-source "Eztv" --url="http://www.ezrss.it/feed/" --source-parser "defaultrss" --metatype "torrent"
```
or
```
$ swarmtv --add-source "Piratebay" --url="http://rss.thepiratebay.org/205" --source-parser "defaultrss" --metatype "torrent"
```

The option --source-parser selects what filter is to be used in order to convert the RSS into database records in the 'newtorrents' table.
The option --metatype tells Swarmtv what kind of meta files to expect (NZB or Torrent).

## Adding simple filters

### Example
```
$ swarmtv --add-simple='House' --nodup='newer' --title='^house' --source='(Eztv|TV Torrents)' --exclude='hunters international' --max-size='700.00 MB' --min-size='200.00 MB'
```

This filter filters for all torrents with the name beginning with "house" and excludes "House hunters international" which we don't want.
To avoid downloading the HD version of the House episodes I added a maximal size of 700Mb, a good quality episode should be at least 200Mb.
The source options provides the user with a way to restrict the RSS sources the download originates from.

The nodup filter states that we only want newer episodes, no reruns for us.

In the man page of Swarmtv, you can find more information on simple filters.

## Adding SQL filters

Filters are used to get the content you want, and avoid duplicate downloads. Filters are made up from 2 parts, the filters part that filters out the candidates for download. And the avoid duplicates part, that looks if a match the filters has already found isn't downloaded before. Both filter parts are sqlite3 queries, you can create them as you like, as long as the following rows are selected for the filter: link, title, pubdate, category, season, episode note: the fields should be selected in this exact order. The duplicate query can select any field it wants because the programs only looks at row count any row count of => 1 indicates a torrent has been downloaded before.

Filters can be tested with the -q option together with the -F and -T option. The query is executed on a sandboxed database, where all 'new' flags are set to 'Y'. After the query has executed, a list of files is printed that would have been downloaded by this rule.
When this list consists of the correct files and no doubles are found, the filter should be okay. By removing the -q option from the command line, the filter can be inserted. Note: for this feature to work relevant data must be present in the newtorrents table.

### Examples of SQL filters

#### Test filter:

```
$ swarmtv **--test** --add-SQL-filter "DollHouse:select link, title, pubdate, category, season, episode 
from newtorrents where title REGEXP('^[Dd]ollhouse') AND size < '400000000' AND new = 'Y'"
--nodup-sql-filter "SELECT title FROM downloaded WHERE link=?1 OR (season=?2 AND episode=?3 AND title REGEXP('^[Dd]ollhouse'))"
```

#### Enter filter:
```
$ swarmtv --add-SQL-filter "DollHouse:select link, title, pubdate, category, season, episode  
from newtorrents where title REGEXP('^[Dd]ollhouse') AND size < '400000000' AND new = 'Y'" --nodup-sql-filter "SELECT title FROM downloaded WHERE link=?1 OR (season=?2 AND episode=?3 AND title REGEXP('^[Dd]ollhouse'))"
```
The --add-sql-filter option expects "<name>:<filter query="">" the name will be the name of the filter.
The filter query selects the rows that match a query.
the rows that are selected in the query are mandatory, the where clause is free to write your self.

The --nodup-sql-filter argument expects <no duplicate="" query=""> when this query returns 1 or more records, the download candidate is considered a duplicate and ignored.
The selected fields are of no influence here,although 1 row should be selected.

For more information or suggestions to this program, Please contact me at paul [at] interweps.nl

## TODO

*   Creating a tool that downloads missed episodes when you create a downloadfilter in the middle of a serie.
*   Add hooks to provide downloading of Torrents and NZB's in the frontend of libSwarmtv
*   GNU gettext internationalization support
*   Clean out library, all errors through a callback API
*   Intergrate Torrent/NZB download libraries

## Contact

For bug reports and feature requests please contact me at swarmtv [at] swarmtv.nl

## GIT
[https://github.com/annejan/swarmtv](https://github.com/annejan/swarmtv)

[https://git.etv.cx/?a=summary&p=swarmtv](https://git.etv.cx/?a=summary&p=swarmtv)

## Disclaimer

This program is free software: you can redistribute it and/or modify it under the terms of the [GNU General Public License](http://swarmtv.nl/gpl.html)as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, SOUL OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
