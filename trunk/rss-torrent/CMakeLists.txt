# The name of our project is "HELLO". CMakeLists files in this project can
# refer to the root source directory of the project as ${HELLO_SOURCE_DIR} and
# to the root binary directory of the project as ${HELLO_BINARY_DIR}.
cmake_minimum_required(VERSION 2.6)
project (rsstorrent C)

# Compiler directives
add_definitions( `xml2-config --cflags` -Wall -W -g)
link_libraries(curl xml2 magic pcre esmtp sqlite3)
include_directories(.)


# RSS filter sources go here
SET(RSSFILTERS 
rssfilter/eztv/eztvfilter 
rssfilter/rsstorrent/rsstorrentfilter)


# All other sources
SET(SRC ${RSSFILTERS} rsstorrent database config regexp source
  filter runloop curlfile torrentdb torrentdownload logfile
  daemonize mailmsg findtorrent mymagic sandboxdb testfilter
  filesystem tbl torrentparse)

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

# Make database
ADD_CUSTOM_COMMAND(
    OUTPUT examplescript
    COMMAND ${rsstorrent_SOURCE_DIR}/examples.sh
    )

ADD_CUSTOM_TARGET( examples DEPENDS examplescript)


ADD_CUSTOM_COMMAND(
    OUTPUT dbscript
    COMMAND ${rsstorrent_SOURCE_DIR}/createdb.sh
    )

ADD_CUSTOM_TARGET( db DEPENDS dbscript)

