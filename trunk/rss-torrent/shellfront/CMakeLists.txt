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
include_directories(. ${RSSTORRENT_BASE_DIR}/librsstor)

# Make sure the linker can find the Hello library once it is built.
link_directories (${RSSTORRENT_BASE_DIR}/librsstor) 

SET(SRC rsstorrent handleopts present frontfuncts daemonize)

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

# Installation of the manuals
INSTALL(FILES man/rsstorrent.1.gz
		DESTINATION "share/man/man1"
		PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

# Add executables / manuals
add_executable (rsstorrent ${SRC} ${RSS_MAN}) 
TARGET_LINK_LIBRARIES(rsstorrent ${optionalLibs} librsstor.so )

# We install to /usr/local/bin
INSTALL(PROGRAMS rsstorrent DESTINATION /usr/local/bin)

