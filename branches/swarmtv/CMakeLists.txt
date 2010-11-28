# The name of our project is "HELLO". CMakeLists files in this project can
# refer to the root source directory of the project as ${HELLO_SOURCE_DIR} and
# to the root binary directory of the project as ${HELLO_BINARY_DIR}.
cmake_minimum_required(VERSION 2.6)
project (swarmtv C)

# Handle build switches
IF(NOT DEFINED ESMTP_ENABLE)
	SET(ESMTP_ENABLE TRUE)
ENDIF()

IF(NOT DEFINED SHELL_ENABLE)
  SET(SHELL_ENABLE TRUE)
ENDIF()

# When the SHELL FRONTEND is disabled, no Dbus Glib intergration is possible.
IF(SHELL_ENABLE)
  IF(NOT DEFINED DBUS_GLIB_ENABLE)
    SET(DBUS_GLIB_ENABLE TRUE)
  ENDIF()
ELSE()
  SET(DBUS_GLIB_ENABLE FALSE)
ENDIF()

# Print options
MESSAGE("## OPTIONS ##")
MESSAGE("ESMTP_ENABLE = ${ESMTP_ENABLE}")
MESSAGE("SHELL_ENABLE = ${SHELL_ENABLE}")
MESSAGE("DBUS_GLIB_ENABLE = ${DBUS_GLIB_ENABLE}")


# Detect Libraries
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} /usr/share/apps/cmake/modules ${CMAKE_SOURCE_DIR}/cmake_modules)

# When DBUS_GLIB is enabled, check the library
IF(DBUS_GLIB_ENABLE)
  FIND_PACKAGE(GLIB2 REQUIRED)
  FIND_PACKAGE(DbusGlib REQUIRED)
ENDIF()

# Other libs
FIND_PACKAGE(LibXml2 REQUIRED)
FIND_PACKAGE(Sqlite REQUIRED)
FIND_PACKAGE(PCRE REQUIRED)
FIND_PACKAGE(CURL REQUIRED)

# When ESMTP is enabled, check the library
IF(ESMTP_ENABLE) 
	FIND_PACKAGE(ESMTP)
	IF(ESMTP_FOUND)
		ADD_DEFINITIONS(-DRSST_ESMTP_ENABLE="\\"1\\"")
	ELSEIF()
		MESSAGE("ESMTP NOT FOUND")
	ENDIF()
ELSEIF()
	SET(ESMTP_FOUND FALSE)
ENDIF()

SET(RSSTORRENT_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Building libray and shell frontend
add_subdirectory (libswarm)

IF(SHELL_ENABLE) 
	add_subdirectory (shellfront) 
ENDIF()

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
		COMMAND find . -name CMakeCache.txt | xargs rm -r 
		COMMAND find . -name CMakeFiles | xargs rm -r
    )

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

