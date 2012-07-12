INCLUDE(InstallFilesRecursive)

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

ADD_LIBRARY(${LIBRARY_NAME} SHARED ${RSBTS_SOURCES} ${RSBTS_HEADERS})
TARGET_LINK_LIBRARIES(${LIBRARY_NAME} ${RSB_LIBRARIES}
                                      ${RST_LIBRARIES}
                                      ${Boost_LIBRARIES})

ADD_EXECUTABLE(${BINARY_NAME} rsbtimesync/rsbtimesync.cpp)
TARGET_LINK_LIBRARIES(${BINARY_NAME} ${LIBRARY_NAME})

# Install rules

INSTALL(TARGETS ${LIBRARY_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

INSTALL(PROGRAMS    "${CMAKE_CURRENT_BINARY_DIR}/${BINARY_NAME}"
        DESTINATION "bin"
        RENAME      "${BINARY_PREFIX}${BINARY_NAME}${BINARY_SUFFIX}")

INSTALL_FILES_RECURSIVE(include RSBTS_HEADERS)
