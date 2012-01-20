# -*- mode: cmake -*-

# Logger binary

FILE(GLOB LOGGER_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")
FILE(GLOB LOGGER_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h")

# There is a bug in boost 1.38, which prevents the stats formatter from working.
# Hence we remove it here if this version is used
IF(Boost_VERSION LESS 103801)
    ADD_DEFINITIONS(-DRSB_LOGGER_NO_STATISTICS_FORMATTER)
    MESSAGE(STATUS "Boost version is too old, disabling statistics formatter due to a bug.")
    LIST(REMOVE_ITEM LOGGER_SOURCES "StatisticsEventFormatter.cpp")
    LIST(REMOVE_ITEM LOGGER_HEADERS "StatisticsEventFormatter.h")
    LIST(REMOVE_ITEM LOGGER_SOURCES "MonitorEventFormatter.cpp")
    LIST(REMOVE_ITEM LOGGER_HEADERS "MonitorEventFormatter.h")
ENDIF()

ADD_EXECUTABLE(logger ${LOGGER_SOURCES} ${LOGGER_HEADERS})

TARGET_LINK_LIBRARIES(logger ${RSC_LIBRARIES}
                             ${RSB_LIBRARIES}
                             ${Boost_LIBRARIES}
                             ${PROTOBUF_LIBRARIES})

# Install target

INSTALL(PROGRAMS    "${CMAKE_CURRENT_BINARY_DIR}/logger"
        DESTINATION "bin"
	RENAME      "${BINARY_PREFIX}logger${BINARY_SUFFIX}")
