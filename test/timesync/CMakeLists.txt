
ENABLE_TESTING()

SET(TEST_RESULT_DIR ${CMAKE_BINARY_DIR}/testresults)
SET(TEST_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

SET(TEST_SPREAD_PORT 4816 CACHE STRING "the port of the spread daemon for tests")
SET(TEST_SPREAD_CONFIG_SYMLINK "" CACHE STRING "A symlink pointing to the generated test spread config file. If not empty it will be created and used for the unit tests. This is required to prevent a size limitation bug for the argument in the spread daemon.")
SET(SPREAD_CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/spread.conf")

SET(REAL_SPREAD_CONFIG_FILE ${SPREAD_CONFIG_FILE})
IF(TEST_SPREAD_CONFIG_SYMLINK)
    SET(REAL_SPREAD_CONFIG_FILE ${TEST_SPREAD_CONFIG_SYMLINK})
ENDIF()

CONFIGURE_FILE(spread.conf.in ${SPREAD_CONFIG_FILE} @ONLY)
CONFIGURE_FILE(testconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/testconfig.h)

INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_BINARY_DIR}
                           "${CMAKE_SOURCE_DIR}/src/timesync"
                           ${GMOCK_INCLUDE_DIRS})

ADD_EXECUTABLE(rsbtimesynctest rsbtimesync/rsbtimesynctest.cpp
                               rsbtimesync/ApproximateTimeStrategyTest.cpp)

TARGET_LINK_LIBRARIES(rsbtimesynctest ${TIMESYNC_LIBRARY_NAME}
                                      ${GMOCK_LIBRARIES})
                                      
ADD_TEST(rsbtimesynctest rsbtimesynctest "--gtest_output=xml:${TEST_RESULT_DIR}/")
