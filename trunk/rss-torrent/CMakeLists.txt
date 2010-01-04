# The name of our project is "HELLO". CMakeLists files in this project can
# refer to the root source directory of the project as ${HELLO_SOURCE_DIR} and
# to the root binary directory of the project as ${HELLO_BINARY_DIR}.
cmake_minimum_required(VERSION 2.6)
project (rsstorrent C)

# Compiler directives
add_definitions( `xml2-config --cflags` -Wall -g)
link_libraries(curl xml2 magic pcre esmtp sqlite3)
include_directories(.)


# RSS filter sources go here
SET(RSSFILTERS rssfilter/eztv/eztvfilter rssfilter/rsstorrent/rsstorrentfilter)

# All other sources
SET(SRC ${RSSFILTERS} rsstorrent.c database.c config.c regexp.c source.c
  filter.c runloop.c curlfile.c torrentdb.c torrentdownload.c logfile.c
  daemonize.c mailmsg.c findtorrent.c mymagic.c sandboxdb.c testfilter.c)

# We install to /usr/local/bin
INSTALL(PROGRAMS rsstorrent DESTINATION /usr/local/bin)

# Add executable
add_executable (
  rsstorrent ${SRC}
) 

CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
    "${CMAKE_COMMAND}" -P
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

