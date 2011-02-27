# Use the detected libraries to set build variables.
include_directories(. ${RSSTORRENT_BASE_DIR}/libswarmtv)
link_directories (${RSSTORRENT_BASE_DIR}/libswarmtv) 

# Add executable
add_executable (winservice_exec swarmtvservice)  

SET_TARGET_PROPERTIES(winservice_exec PROPERTIES OUTPUT_NAME "swarmtvservice")

TARGET_LINK_LIBRARIES(winservice_exec ${optionalLibs} libswarmtv.dll )

