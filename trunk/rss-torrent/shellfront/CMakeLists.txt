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
include_directories(. ../librsstor)

# Make sure the linker can find the Hello library once it is built.
link_directories (../librsstor) 

SET(SRC rsstorrent handleopts present frontfuncts daemonize)

# Add executable
add_executable (rsstorrent ${SRC}) 
TARGET_LINK_LIBRARIES(rsstorrent ${optionalLibs} librsstor.so )

# We install to /usr/local/bin
INSTALL(PROGRAMS rsstorrent DESTINATION /usr/local/bin)

