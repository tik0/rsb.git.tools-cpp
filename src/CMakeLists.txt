
PROTOBUF_GENERATE(PROTOFILES "${CMAKE_CURRENT_SOURCE_DIR}/rsbtimesync/SyncMap.proto"
                  PROTOROOT "${CMAKE_CURRENT_SOURCE_DIR}/rsbtimesync"
                  OUTPATH "${CMAKE_CURRENT_BINARY_DIR}/rsbtimesync-generated"
                  INCLUDES ${RSBPROTO_ROOT}
                  CPP
                  PROTO_SOURCES PROTO_HEADERS)

INCLUDE_DIRECTORIES(BEFORE "${CMAKE_CURRENT_BINARY_DIR}/rsbtimesync-generated")

SET(RSBTS_SOURCES rsbtimesync/ApproximateTimeStrategy.cpp
                  rsbtimesync/FirstMatchStrategy.cpp
                  rsbtimesync/InformerHandler.cpp
                  rsbtimesync/SchemaAndByteArrayConverter.cpp
                  rsbtimesync/SyncDataHandler.cpp
                  rsbtimesync/SyncMapConverter.cpp
                  rsbtimesync/SyncStrategy.cpp
                  rsbtimesync/TimeFrameStrategy.cpp
                  ${PROTO_SOURCES})

SET(RSBTS_HEADERS rsbtimesync/ApproximateTimeStrategy.h
                  rsbtimesync/FirstMatchStrategy.h
                  rsbtimesync/InformerHandler.h
                  rsbtimesync/SchemaAndByteArrayConverter.h
                  rsbtimesync/SyncDataHandler.h
                  rsbtimesync/SyncMapConverter.h
                  rsbtimesync/SyncStrategy.h
                  rsbtimesync/TimeFrameStrategy.h
                  ${PROTO_HEADERS})

ADD_LIBRARY(${LIBRARY_NAME} SHARED ${RSBTS_SOURCES} ${RSBTS_HEADERS})
TARGET_LINK_LIBRARIES(${LIBRARY_NAME} ${RSB_LIBRARIES}
                                      ${RST_LIBRARIES}
                                      ${Boost_LIBRARIES})

ADD_EXECUTABLE(${BINARY_NAME} rsbtimesync/rsbtimesync.cpp)
TARGET_LINK_LIBRARIES(${BINARY_NAME} ${LIBRARY_NAME})
