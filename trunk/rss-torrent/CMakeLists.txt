# The name of our project is "HELLO". CMakeLists files in this project can
# refer to the root source directory of the project as ${HELLO_SOURCE_DIR} and
# to the root binary directory of the project as ${HELLO_BINARY_DIR}.
cmake_minimum_required(VERSION 2.6)
project (rsstorrent C)

# Handle ESMTP ENABLE
#IF(NOT ESMTP_ENABLE)
#	SET(ESMTP_ENABLE "NO")
#ENDIF(ESMTP_ENABLE)
# MESSAGE("ESMTP_ENABLE=${ESMTP_ENABLE}")

# Detect Libraries
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} /usr/share/apps/cmake/modules ${CMAKE_SOURCE_DIR}/cmake_modules)
FIND_PACKAGE(LibXml2 REQUIRED)
FIND_PACKAGE(Sqlite REQUIRED)
FIND_PACKAGE(PCRE REQUIRED)
FIND_PACKAGE(CURL REQUIRED)

# When ESMTP is enabled, check the library
#IF("${ESMTP_ENABLE}" STREQUAL "YES") 
	FIND_PACKAGE(ESMTP REQUIRED)
#ENDIF("${ESMTP_ENABLE}" EQUAL "YES")


# Add manpage 
SET(RSS_SRC man/rsstorrent.1)
SET(RSS_MAN man/rsstorrent.1.gz)

# Compression of the manuals
FOREACH(man ${RSS_SRC})
  ADD_CUSTOM_COMMAND(OUTPUT ${man}.gz
      COMMAND gzip -c ${man} > ${man}.gz
      DEPENDS ${man}
      COMMENT
      "Building
      ${man}.gz")
ENDFOREACH(man)

# Building libray and shell frontend
add_subdirectory (librsstor)
add_subdirectory (shellfront) 

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

