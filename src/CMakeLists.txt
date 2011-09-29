
PROTOBUF_GENERATE(PROTOFILES "${CMAKE_CURRENT_SOURCE_DIR}/rsbtimesync/SyncMap.proto"
                  PROTOROOT "${CMAKE_CURRENT_SOURCE_DIR}/rsbtimesync"
                  OUTPATH "${CMAKE_CURRENT_BINARY_DIR}/rsbtimesync-generated"
                  CPP
                  PROTO_SOURCES PROTO_HEADERS)

INCLUDE_DIRECTORIES(BEFORE "${CMAKE_CURRENT_BINARY_DIR}/rsbtimesync-generated")

SET(RSBTS_SOURCES rsbtimesync/rsbtimesync.cpp
                  rsbtimesync/SchemaAndByteArrayConverter.cpp
                  rsbtimesync/SyncMapConverter.cpp
                  ${PROTO_SOURCES})

SET(RSBTS_HEADERS rsbtimesync/SyncMapConverter.h
                  rsbtimesync/SchemaAndByteArrayConverter.h
                  ${PROTO_HEADERS})

ADD_EXECUTABLE(rsbtimesync ${RSBTS_SOURCES} ${RSBTS_HEADERS})
TARGET_LINK_LIBRARIES(rsbtimesync ${RSB_LIBRARIES}
                                  ${RST_LIBRARIES}
                                  ${Boost_LIBRARIES})
