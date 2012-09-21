
SET(LIB_SOURCES rsb/tools/buffer/Buffer.cpp
                rsb/tools/buffer/BufferInsertHandler.cpp
                rsb/tools/buffer/BufferRequestCallback.cpp
                rsb/tools/buffer/TimeBoundedBuffer.cpp)

SET(LIB_HEADERS rsb/tools/buffer/Buffer.h
                rsb/tools/buffer/BufferInsertHandler.h
                rsb/tools/buffer/BufferRequestCallback.h
                rsb/tools/buffer/TimeBoundedBuffer.h)

ADD_LIBRARY(${BUFFER_LIBRARY_NAME} SHARED ${LIB_SOURCES} ${LIB_HEADERS})
TARGET_LINK_LIBRARIES(${BUFFER_LIBRARY_NAME} ${RSB_LIBRARIES})
SET_TARGET_PROPERTIES(${BUFFER_LIBRARY_NAME}
                      PROPERTIES
                      VERSION ${SO_VERSION})

SET(BIN_SOURCES rsb/tools/buffer/simple_runner.cpp)

SET(BIN_HEADERS )

ADD_EXECUTABLE(${BUFFER_BINARY_NAME} ${BIN_SOURCES} ${BIN_HEADERS})
TARGET_LINK_LIBRARIES(${BUFFER_BINARY_NAME} ${BUFFER_LIBRARY_NAME})

INSTALL(TARGETS ${BUFFER_LIBRARY_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

INSTALL(PROGRAMS    "${CMAKE_CURRENT_BINARY_DIR}/${BUFFER_BINARY_NAME}"
        DESTINATION "bin"
        RENAME      "${BINARY_PREFIX}${BUFFER_BINARY_NAME}${BINARY_SUFFIX}")
        