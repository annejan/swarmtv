# The name of our project is "HELLO". CMakeLists files in this project can
# refer to the root source directory of the project as ${HELLO_SOURCE_DIR} and
# to the root binary directory of the project as ${HELLO_BINARY_DIR}.
cmake_minimum_required(VERSION 2.6)
project (swarmtv C)

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
if (UNIX AND NOT WIN32)
	add_definitions(-Wall -W -fPIC)
else (WIN32)
	add_definitions(-Wall -W -DPIC -DDLL_EXPORT)	
endif (UNIX AND NOT WIN32)
IF(DEBUG_ENABLE)
  add_definitions(-g)
ENDIF()
include_directories(.)

# All other sources
# getmissed callbackimpl 
if (UNIX AND NOT WIN32)
SET(LIBSRC setup database databaseimpl config regexp source
		filter runloop curlfile torrentdb torrentdownload logfile
		sandboxdb testfilter filesystem lastdownloaded 
		simplefilter callback stats baretitle)
else (WIN32)
SET(LIBSRC setup database databaseimpl config regexp source workarounds
		filter runloop curlfile torrentdb torrentdownload logfile
		sandboxdb testfilter filesystem lastdownloaded 
		simplefilter callback stats baretitle)
endif (UNIX AND NOT WIN32)

# Subdirectories
INCLUDE(srcparser/CMakeLists.txt)
INCLUDE(filehandler/CMakeLists.txt)

# Add library
add_library (swarmtv SHARED ${LIBSRC})
TARGET_LINK_LIBRARIES(swarmtv ${optionalLibs} )
# Set the build version (VERSION) and the API version (SOVERSION)
SET_TARGET_PROPERTIES(swarmtv
		PROPERTIES
		VERSION 1.0.0
		SOVERSION 1)

# Generating pkg-config file
EXEC_PROGRAM( libswarmtv/createpc.sh ARGS ${CMAKE_INSTALL_PREFIX} )

# Installation of the library
INSTALL(TARGETS swarmtv
		DESTINATION lib
		PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

INSTALL(FILES swarmtv.h
		DESTINATION include
		PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

INSTALL(FILES libswarmtv.pc DESTINATION lib/pkgconfig/ )
