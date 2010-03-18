# The name of our project is "HELLO". CMakeLists files in this project can
# refer to the root source directory of the project as ${HELLO_SOURCE_DIR} and
# to the root binary directory of the project as ${HELLO_BINARY_DIR}.
cmake_minimum_required(VERSION 2.6)
project (rsstorrent C)

# Detect Libraries
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} /usr/share/apps/cmake/modules ${CMAKE_SOURCE_DIR}/cmake_modules)
FIND_PACKAGE(LibXml2 REQUIRED)
FIND_PACKAGE(Sqlite REQUIRED)
FIND_PACKAGE(PCRE REQUIRED)
FIND_PACKAGE(CURL REQUIRED)
FIND_PACKAGE(ESMTP REQUIRED)

# Use the detected libraries to set build variables.
SET(optionalLibs)
IF(LIBXML2_FOUND)
	INCLUDE_DIRECTORIES( ${LIBXML2_INCLUDE_DIR} )
	SET(optionalLibs ${optionalLibs} ${LIBXML2_LIBRARIES} )
ENDIF(LIBXML2_FOUND)

IF(SQLITE_FOUND)
	INCLUDE_DIRECTORIES( ${SQLITE_INCLUDE_DIR} )
	SET(optionalLibs ${optionalLibs} ${SQLITE_LIBRARIES} )
ENDIF(SQLITE_FOUND)

IF(PCRE_FOUND)
	INCLUDE_DIRECTORIES( ${PCRE_INCLUDE_DIR} )
	SET(optionalLibs ${optionalLibs} ${PCRE_LIBRARIES} )
ENDIF(PCRE_FOUND)

IF(CURL_FOUND)
	INCLUDE_DIRECTORIES( ${CURL_INCLUDE_DIR} )
	SET(optionalLibs ${optionalLibs} ${CURL_LIBRARIES} )
ENDIF(CURL_FOUND)

IF(ESMTP_FOUND)
	INCLUDE_DIRECTORIES( ${ESMTP_INCLUDE_DIR} )
	SET(optionalLibs ${optionalLibs} ${ESMTP_LIBRARIES} )
ENDIF(ESMTP_FOUND)

# Compiler directives
add_definitions(-Wall -W -g)
include_directories(.)

# Add manpage 
SET(RSS_SRC man/rsstorrent.1)
SET(RSS_MAN man/rsstorrent.1.gz)

# All other sources
SET(SRC setup rsstorrent database config regexp source
  filter runloop curlfile torrentdb torrentdownload logfile
  daemonize mailmsg findtorrent sandboxdb testfilter
  filesystem tbl torrentparse handleopts simplefilter)

# Subdirectories
INCLUDE(srcparser/parsers.cmake)

# Add executable
add_executable (rsstorrent ${SRC} ${RSS_MAN}) 
TARGET_LINK_LIBRARIES(rsstorrent ${optionalLibs} )

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
		COMMAND rm CMakeCache.txt
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

