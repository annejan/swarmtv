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

IF(DBUSGLIB_FOUND)
  # Insert the DBUS_GLIB library and include definitions
  INCLUDE_DIRECTORIES( ${DBUSGLIB_INCLUDE_DIRS} )
  INCLUDE_DIRECTORIES( ${DBUS_INCLUDE_DIR} )
  INCLUDE_DIRECTORIES( ${DBUS_ARCH_INCLUDE_DIR} )
  INCLUDE_DIRECTORIES( ${GLIB2_INCLUDE_DIR} )
  INCLUDE_DIRECTORIES( ${GLIBCONFIG_INCLUDE_DIR} )
  SET(optionalLibs ${optionalLibs} ${GLIB2_LIBRARIES} )
  SET(optionalLibs ${optionalLibs} ${DBUSGLIB_LIBRARY} )
  ADD_DEFINITIONS(-DRSST_DBUS_GLIB_ENABLE="\\"1\\"")
ENDIF(DBUSGLIB_FOUND)

# Compiler directives
add_definitions(-Wall -W -g)
include_directories(. ${RSSTORRENT_BASE_DIR}/libswarm)

# Make sure the linker can find the Hello library once it is built.
link_directories (${RSSTORRENT_BASE_DIR}/libswarm) 

# Add the sources to be build
SET(SRC swarmtv handleopts present frontfuncts daemonize simplewizard runloop xmlencode)

# When dbusglib is available add the dbus source
IF(DBUSGLIB_FOUND)
  SET(SRC ${SRC} dbus)
ENDIF(DBUSGLIB_FOUND)

# Add manpage 
SET(RSS_SRC man/swarmtv.1)
SET(RSS_MAN man/swarmtv.1.gz)

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
INSTALL(FILES man/swarmtv.1.gz
		DESTINATION "share/man/man1"
		PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

# Add executables / manuals
add_executable (swarmtv ${SRC} ${RSS_MAN}) 
TARGET_LINK_LIBRARIES(swarmtv ${optionalLibs} libswarm.so )

# We install to /usr/local/bin
INSTALL(PROGRAMS swarmtv DESTINATION /usr/local/bin)

