
SET(LIB_SOURCES rsbbuffer/Buffer.cpp
                rsbbuffer/BufferInsertHandler.cpp
                rsbbuffer/BufferRequestCallback.cpp
                rsbbuffer/TimeBoundedBuffer.cpp)

SET(LIB_HEADERS rsbbuffer/Buffer.h
                rsbbuffer/BufferInsertHandler.h
                rsbbuffer/BufferRequestCallback.h
                rsbbuffer/TimeBoundedBuffer.h)

ADD_LIBRARY(${BUFFER_LIBRARY_NAME} SHARED ${LIB_SOURCES} ${LIB_HEADERS})
TARGET_LINK_LIBRARIES(${BUFFER_LIBRARY_NAME} ${RSB_LIBRARIES})
SET_TARGET_PROPERTIES(${BUFFER_LIBRARY_NAME}
                      PROPERTIES
                      VERSION ${SO_VERSION})

SET(BIN_SOURCES rsbbuffer/simple_runner.cpp)

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
        