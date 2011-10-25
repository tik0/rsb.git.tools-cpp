
SET(TEST_RESULT_DIR ${CMAKE_BINARY_DIR}/testresults)

INCLUDE_DIRECTORIES(BEFORE ../src
                           ${CMAKE_CURRENT_BINARY_DIR}/../src
                           ${GTEST_INCLUDE_DIR}
                           ${GMOCK_INCLUDE_DIR})

ADD_EXECUTABLE(rsbtimesynctest rsbtimesync/rsbtimesynctest.cpp
                               rsbtimesync/ApproximateTimeStrategyTest.cpp)

TARGET_LINK_LIBRARIES(rsbtimesynctest ${LIBRARY_NAME}
                                      ${GMOCK_LIBRARY_NAME})
                                      
ADD_TEST(rsbtimesynctest rsbtimesynctest "--gtest_output=xml:${TEST_RESULT_DIR}/")
