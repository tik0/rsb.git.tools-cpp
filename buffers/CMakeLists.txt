
# --- global definitions ---

SET(RSBBUFFER_LIB_NAME "rsbbuffer")
SET(RSBBUFFER_BIN_NAME "buffer")

# --- source code ---

ADD_SUBDIRECTORY(src)

IF(OPTION_BUILD_EXAMPLES)
    ADD_SUBDIRECTORY(examples)
ENDIF()

ENABLE_TESTING()
IF(OPTION_BUILD_TESTS AND GMOCK_AVAILABLE)
    ADD_SUBDIRECTORY(test)
ENDIF()

# --- documentation generation ---

GENERATE_DOXYGEN(VERSION "${RSB_TOOLS_VERSION_MAJOR}.${RSB_TOOLS_VERSION_MINOR}.${RSB_TOOLS_VERSION_PATCH}")

# --- coverage ---

#ENABLE_COVERAGE_REPORT(TARGETS ${RSBBUFFER_LIB_NAME} TESTS ${RSC_TEST_NAME} FILTER "*3rdparty*" "*test/*")

# --- sloccount ---

ENABLE_SLOCCOUNT(FOLDERS src)

# --- cppcheck ---

GENERATE_CPPCHECK(SOURCES src test examples
                          "${CMAKE_CURRENT_BINARY_DIR}/src" "${CMAKE_CURRENT_BINARY_DIR}/test" "${CMAKE_CURRENT_BINARY_DIR}/examples"
                  ENABLE_IDS style
                  INLINE_SUPPRESSION)
