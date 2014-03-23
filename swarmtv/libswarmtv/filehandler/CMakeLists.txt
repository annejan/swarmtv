# The plugins should be inserted here.
# When adding parsers here, make sure to add the parser in parser.c to.
SET(FILEHANDLE_PATH filehandler)

SET(LIBSRC ${LIBSRC} 
  ${FILEHANDLE_PATH}/filehandler
)

INCLUDE(filehandler/torrent/CMakeLists.txt)
INCLUDE(filehandler/nzb/CMakeLists.txt)
