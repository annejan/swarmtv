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
add_definitions(-Wall -W -g -fPIC)
include_directories(.)

# All other sources
# getmissed
SET(LIBSRC setup database databaseimpl config regexp source
		filter runloop curlfile torrentdb torrentdownload logfile
		sandboxdb testfilter filesystem lastdownloaded 
		simplefilter callback callbackimpl stats baretitle)

IF(ESMTP_FOUND)
	SET(LIBSRC ${LIBSRC} mailmsg)
ENDIF()

# Subdirectories
INCLUDE(srcparser/CMakeLists.txt)
INCLUDE(filehandler/CMakeLists.txt)

# Add library
add_library (swarm SHARED ${LIBSRC})
TARGET_LINK_LIBRARIES(swarm ${optionalLibs} )
# Set the build version (VERSION) and the API version (SOVERSION)
SET_TARGET_PROPERTIES(swarm
		PROPERTIES
		VERSION 1.1.0
		SOVERSION 1)

# Installation of the library
INSTALL(TARGETS swarm
		DESTINATION lib
		PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

INSTALL(FILES swarm.h
		DESTINATION include
		PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

# Implement uninstall
#CONFIGURE_FILE(
#    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
#    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
#    IMMEDIATE @ONLY)

#ADD_CUSTOM_TARGET(uninstall
#    "${CMAKE_COMMAND}" -P
#    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

#ADD_CUSTOM_TARGET(realclean
#		make clean
#		COMMAND rm CMakeCache.txt
#		COMMAND rm -rf CMakeFiles )

