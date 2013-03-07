
SET(RSBTS_SOURCES rsb/tools/timesync/ApproximateTimeStrategy.cpp
                  rsb/tools/timesync/FirstMatchStrategy.cpp
                  rsb/tools/timesync/InformerHandler.cpp
                  rsb/tools/timesync/PriorityTimestampSelector.cpp
                  rsb/tools/timesync/StaticTimestampSelectors.cpp
                  rsb/tools/timesync/SyncDataHandler.cpp
                  rsb/tools/timesync/SyncStrategy.cpp
                  rsb/tools/timesync/TimeFrameStrategy.cpp
                  rsb/tools/timesync/TimestampSelector.cpp)

SET(RSBTS_HEADERS rsb/tools/timesync/ApproximateTimeStrategy.h
                  rsb/tools/timesync/FirstMatchStrategy.h
                  rsb/tools/timesync/InformerHandler.h
                  rsb/tools/timesync/PriorityTimestampSelector.h
                  rsb/tools/timesync/StaticTimestampSelectors.h
                  rsb/tools/timesync/SyncDataHandler.h
                  rsb/tools/timesync/SyncStrategy.h
                  rsb/tools/timesync/TimeFrameStrategy.h
                  rsb/tools/timesync/TimestampSelector.h)

ADD_LIBRARY(${TIMESYNC_LIBRARY_NAME} SHARED ${RSBTS_SOURCES} ${RSBTS_HEADERS})
TARGET_LINK_LIBRARIES(${TIMESYNC_LIBRARY_NAME} ${RSB_LIBRARIES}
                                      ${RST_LIBRARIES}
                                      ${Boost_LIBRARIES})
SET_TARGET_PROPERTIES(${TIMESYNC_LIBRARY_NAME}
                      PROPERTIES
                      VERSION ${SO_VERSION})

ADD_EXECUTABLE(${TIMESYNC_BINARY_NAME} rsb/tools/timesync/rsbtimesync.cpp)
TARGET_LINK_LIBRARIES(${TIMESYNC_BINARY_NAME} ${TIMESYNC_LIBRARY_NAME})

# Install rules

INSTALL(TARGETS ${TIMESYNC_LIBRARY_NAME}
        EXPORT RSBTSDepends
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

INSTALL(PROGRAMS    "${CMAKE_CURRENT_BINARY_DIR}/${TIMESYNC_BINARY_NAME}"
        DESTINATION "bin"
        RENAME      "${BINARY_PREFIX}${TIMESYNC_BINARY_NAME}${BINARY_SUFFIX}")

INSTALL_FILES_RECURSIVE(include RSBTS_HEADERS)

CONFIGURE_FILE(RSBTSConfig.cmake.in "${CMAKE_BINARY_DIR}/RSBTSConfig.cmake" @ONLY)
INSTALL(FILES "${CMAKE_BINARY_DIR}/RSBTSConfig.cmake" DESTINATION share/RSBTS)
CONFIGURE_FILE(RSBTSConfigVersion.cmake.in "${CMAKE_BINARY_DIR}/RSBTSConfigVersion.cmake" @ONLY)
INSTALL(FILES "${CMAKE_BINARY_DIR}/RSBTSConfigVersion.cmake" DESTINATION share/RSBTS)

EXPORT(TARGETS ${TIMESYNC_LIBRARY_NAME} FILE "${CMAKE_BINARY_DIR}/RSBTSDepends.cmake")
INSTALL(EXPORT RSBTSDepends
        DESTINATION "share/RSBTS")
