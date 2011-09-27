
SET(RSBTS_SOURCES rsbtimesync/rsbtimesync.cpp)

SET(RSBTS_HEADERS )

ADD_EXECUTABLE(rsbtimesync ${RSBTS_SOURCES} ${RSBTS_HEADERS})
TARGET_LINK_LIBRARIES(rsbtimesync ${RSB_LIBRARIES}
                                  ${RST_LIBRARIES}
                                  ${Boost_LIBRARIES})
