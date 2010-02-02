# The name of our project is "HELLO". CMakeLists files in this project can
# refer to the root source directory of the project as ${HELLO_SOURCE_DIR} and
# to the root binary directory of the project as ${HELLO_BINARY_DIR}.
cmake_minimum_required(VERSION 2.6)
project (rsstorrent C)

# Compiler directives
add_definitions( `xml2-config --cflags` -Wall -W -g)
link_libraries(curl xml2 magic pcre esmtp sqlite3)
include_directories(.)

# Add manpage 
SET(RSS_SRC man/rsstorrent.1)
SET(RSS_MAN man/rsstorrent.1.gz)

# All other sources
SET(SRC rsstorrent database config regexp source
  filter runloop curlfile torrentdb torrentdownload logfile
  daemonize mailmsg findtorrent mymagic sandboxdb testfilter
  filesystem tbl torrentparse handleopts simplefilter)

# Subdirectories
INCLUDE(rssparser/parsers.cmake)

# Add executable
  add_executable (
  rsstorrent ${SRC} ${RSS_MAN}
) 

# Compression of the manuals
FOREACH(man ${RSS_SRC})
  ADD_CUSTOM_COMMAND(OUTPUT ${man}.gz
      COMMAND gzip -c ${man} > ${man}.gz
      DEPENDS ${man}
      COMMENT
      "Building
      ${man}.gz")
ENDFOREACH(man)

# We install to /usr/local/bin
INSTALL(PROGRAMS rsstorrent DESTINATION /usr/local/bin)

# Installation of the manuals
  INSTALL(FILES man/rsstorrent.1.gz
      DESTINATION "share/man/man1"
      PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

# Implement uninstall
CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
    "${CMAKE_COMMAND}" -P
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

ADD_CUSTOM_TARGET(realclean
		make clean
		COMMAND rm -rf CMakeFiles )


# Make database and add examples script
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

