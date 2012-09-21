
SET(RSBTS_SOURCES rsbtimesync/ApproximateTimeStrategy.cpp
                  rsbtimesync/FirstMatchStrategy.cpp
                  rsbtimesync/InformerHandler.cpp
                  rsbtimesync/PriorityTimestampSelector.cpp
                  rsbtimesync/StaticTimestampSelectors.cpp
                  rsbtimesync/SyncDataHandler.cpp
                  rsbtimesync/SyncStrategy.cpp
                  rsbtimesync/TimeFrameStrategy.cpp
                  rsbtimesync/TimestampSelector.cpp)

SET(RSBTS_HEADERS rsbtimesync/ApproximateTimeStrategy.h
                  rsbtimesync/FirstMatchStrategy.h
                  rsbtimesync/InformerHandler.h
                  rsbtimesync/PriorityTimestampSelector.h
                  rsbtimesync/StaticTimestampSelectors.h
                  rsbtimesync/SyncDataHandler.h
                  rsbtimesync/SyncStrategy.h
                  rsbtimesync/TimeFrameStrategy.h
                  rsbtimesync/TimestampSelector.h)

ADD_LIBRARY(${TIMESYNC_LIBRARY_NAME} SHARED ${RSBTS_SOURCES} ${RSBTS_HEADERS})
TARGET_LINK_LIBRARIES(${TIMESYNC_LIBRARY_NAME} ${RSB_LIBRARIES}
                                      ${RST_LIBRARIES}
                                      ${Boost_LIBRARIES})
SET_TARGET_PROPERTIES(${TIMESYNC_LIBRARY_NAME}
                      PROPERTIES
                      VERSION ${SO_VERSION})

ADD_EXECUTABLE(${TIMESYNC_BINARY_NAME} rsbtimesync/rsbtimesync.cpp)
TARGET_LINK_LIBRARIES(${TIMESYNC_BINARY_NAME} ${TIMESYNC_LIBRARY_NAME})

# Install rules

INSTALL(TARGETS ${TIMESYNC_LIBRARY_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

INSTALL(PROGRAMS    "${CMAKE_CURRENT_BINARY_DIR}/${TIMESYNC_BINARY_NAME}"
        DESTINATION "bin"
        RENAME      "${BINARY_PREFIX}${TIMESYNC_BINARY_NAME}${BINARY_SUFFIX}")

INSTALL_FILES_RECURSIVE(include RSBTS_HEADERS)
