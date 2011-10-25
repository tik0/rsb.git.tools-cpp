
ENABLE_TESTING()

INCLUDE_DIRECTORIES(BEFORE ../src
                           ${CMAKE_CURRENT_BINARY_DIR}/../src
                           ${GTEST_INCLUDE_DIR}
                           ${GMOCK_INCLUDE_DIR})

ADD_EXECUTABLE(rsbtimesynctest rsbtimesync/rsbtimesynctest.cpp
                               rsbtimesync/ApproximateTimeStrategyTest.cpp)

TARGET_LINK_LIBRARIES(rsbtimesynctest ${LIBRARY_NAME}
                                      ${GMOCK_LIBRARY_NAME})
