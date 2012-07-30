
INCLUDE_DIRECTORIES(BEFORE ../src)

ADD_EXECUTABLE(rsbts_display rsbts_display.cpp)
TARGET_LINK_LIBRARIES(rsbts_display ${LIBRARY_NAME})
