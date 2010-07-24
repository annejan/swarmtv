Introduction :

RSS-torrent is a program that downloads selected torrents from an RSS stream, and puts them in a
directory for your torrent program to find them.
When RSS information is downloaded, the usefull fields are extracted and
inserted into an Sqlite database.
The information that is stored in the Sqlite db is queried using SQL
statements, called parser in RSS-torrent.
When a match is found on one of the filters, a second query against the table holding the downloaded torrents can be used to avoid duplicates.
After both queries are passed, the torrent is downloaded into the "Torrent
directory" specified in the configuration settings.
A torrent client is set up to watch the directory, and pick up the torrent as soon as it arrives.
At this moment I use Rtorrent which does a good job and removes finished
torrents. (http://libtorrent.rakshasa.no)

Build this program :

# cmake .
# make

# make install

To create some example filters and sources in the database

# ./examples.sh

When you want to install rsstorrent

# make install

For examples look at the examples.sh file

Adding sources:

Example

rss source
./rsstorrent -s "Eztv:http://www.ezrss.it/feed/" -t "defaultrss"

Twitter
./rsstorrent -s "Twitter://<username>:<password>@twitter.com/statuses/friends_timeline.xml" -t "twitter"

The first option -s tells rsstorrent to expect an <name>:<url> combination.
the first ':' that is found in the sctring is the delimiter, any ':''s after that are included in the url

The second option the -t selects what parser is to be used in order to convert the rss into database records in the 'newtorrents' table.

Fields in database:

The database within rss-torrent contains 2 tables that are important when
filters are created. 
The first table is the newtorrents table, this table
contains all torrents that passed the parser from source RSSes. The use of the
table is to look for new programs to download.
The second table is the downloaded table, all relevant information of the 
torrent that is being passed to the torrent-program is stored here. The
primary use of the table is to avoid duplicate downloads.


- Table new torrents
id INTEGER PRIMARY KEY    - The id of the record
title TEXT                - The Title of the show 
link TEXT UNIQUE          - The URL to the torrent
pubdate DATE              - The Date the show was released 
category TEXT             - The category the show belongs int
season INTEGER            - The season number of the show
episode INTEGER           - The episode number of the show
seeds INTEGER DEFAULT 0   - The number of seeds associated with the torrent
peers INTEGER DEFAULT 0   - The number of peers associated with the torrent
size INTEGER              - The size of the torrent in bytes
new TEXT DEFAULT 'Y'      - This value is 'Y' when first parsed, after that 'N'


- Table downloaded
id INTEGER PRIMARY KEY    - The id of the records
title TEXT                - The title of the show
link TEXT UNIQUE          - The URL to the torrent
pubdate DATE              - The date the torrent was released
category TEXT             - The category the show belongs in
season INTEGER            - The season number
episode INTEGER           - The episode number
date DATE                 - The Date the torrent was downloaded

Adding simple filters:

In order to filter series without writing your own SQL, you can use simple
filters. The filters consists of the following keywords.
add-simple 	: Add a simple filter, as an argument provide the name of the filter.
title				: The regexp for things that HAVE to be in the title.
exclude			: The regexp for things that MUST NOT be in the title.
max-size		: The maximal size of the torrent content.
min-size		: The minimal size of the torrent content.
nodup				: The Kind of duplicate avoiding filter we want (will be explained below)
from-season	:	From which season to download.
from-episode: From which episode in that season to download.	

When adding a simple filter a title and nodup value are mandatory.
Settings that are not needed may be omitted.

nodup filters :
none		: No filtering at all
link		:	No torrents with the same URL get downloaded.
unique	:	No torrents with the same season and episode number get downloaded.
newer		:	Only newer torrents get downloaded.

Adding SQL filters:

SQL Filters are used to get the content you want, and avoid duplicate downloads.
SQL Filters are made up from 2 parts, the filters part that filters out the
candidates for download. And the avoid duplicates part, that looks if a match
the filters has already found isn't downloaded before. 
Both filter parts are sqlite queries, you can create them as you like, 
as long as the following rows are selected for the filter:
link, title, pubdate, category, season, episode
note: the fields should be selected in this exact order.
The duplicate query can select any field it wants because the programs only
looks at row count any row count of => 1 indicates a torrent has been
downloaded before. 

Examples
./rsstorrent -F "DollHouse:select link, title, pubdate, category, season, episode from newtorrents where title REGEXP('^[Dd]ollhouse') AND size < '400000000' AND new = 'Y'" \
  -T "SELECT title FROM downloaded WHERE link=?1 OR (season=?2 AND episode=?3 AND title REGEXP('^[Dd]ollhouse'))"

The -F option expects "<name:filter query>" the name will be the name of the filter.
The filter query selects the rows that match a query.
the rows that are selected in the query are mandatory, the where clause is free to write your self.

The -T argument expects <no duplicate query> when this query returns 1 or more records, the download candidate is considered a duplicate and ignored.
The selected fields are of no influence here, although 1 row should be selected.

For more information or suggestions to this program, Please contact me at 
paul [at] interweps.nl

Disclaimer:

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, SOUL OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
