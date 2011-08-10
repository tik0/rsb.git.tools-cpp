# -*- mode: cmake -*-

# Logger binary

FILE(GLOB LOGGER_SOURCES "*.cpp")

ADD_EXECUTABLE(rsblogger ${LOGGER_SOURCES})

TARGET_LINK_LIBRARIES(rsblogger ${RSC_LIBRARIES} ${RSB_LIBRARIES} ${Boost_LIBRARIES} ${PROTOBUF_LIBRARIES})

# Install target

INSTALL(TARGETS rsblogger
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib)
