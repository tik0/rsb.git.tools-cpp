
SET(LIB_SOURCES rsbbuffer/Buffer.cpp
                rsbbuffer/BufferInsertHandler.cpp
                rsbbuffer/BufferRequestCallback.cpp
                rsbbuffer/TimeBoundedBuffer.cpp)

SET(LIB_HEADERS rsbbuffer/Buffer.h
                rsbbuffer/BufferInsertHandler.h
                rsbbuffer/BufferRequestCallback.h
                rsbbuffer/TimeBoundedBuffer.h)

ADD_LIBRARY(${RSBBUFFER_LIB_NAME} SHARED ${LIB_SOURCES} ${LIB_HEADERS})
TARGET_LINK_LIBRARIES(${RSBBUFFER_LIB_NAME} ${RSB_LIBRARIES})

SET(BIN_SOURCES rsbbuffer/simple_runner.cpp)

SET(BIN_HEADERS )

ADD_EXECUTABLE(${RSBBUFFER_BIN_NAME} ${BIN_SOURCES} ${BIN_HEADERS})
TARGET_LINK_LIBRARIES(${RSBBUFFER_BIN_NAME} ${RSBBUFFER_LIB_NAME})
