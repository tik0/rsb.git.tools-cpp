CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)

PROJECT("rsb-buffer")

FIND_PACKAGE(RSC REQUIRED)
LIST(INSERT CMAKE_MODULE_PATH 0 ${RSC_CMAKE_MODULE_PATH})

INCLUDE(InstallFilesRecursive)
INCLUDE(EnableCoverageReport)
INCLUDE(EnableSlocCount)
INCLUDE(GenerateDoxygen)
INCLUDE(GenerateCppcheck)

# --- user options ---

OPTION(BUILD_TESTS "Decide if the test suite shall be built or not" ON)

# --- global definitions ---

# version information
SET(RSBBUFFER_VERSION_MAJOR "0")
SET(RSBBUFFER_VERSION_MINOR "1")
SET(RSBBUFFER_VERSION_PATCH "0")
SET(RSBBUFFER_VERSION "${RSBBUFFER_VERSION_MAJOR}.${RSBBUFFER_VERSION_MINOR}.${RSBBUFFER_VERSION_PATCH}")

SET(GTEST_DIR "${CMAKE_SOURCE_DIR}/3rdparty/gmock/gtest")
SET(GTEST_INCLUDE_DIR "${GTEST_DIR}/include")
SET(GMOCK_DIR "${CMAKE_SOURCE_DIR}/3rdparty/gmock")
SET(GMOCK_INCLUDE_DIR "${GMOCK_DIR}/include")
SET(GMOCK_LIBRARY_NAME "gmock")

SET(RSBBUFFER_LIB_NAME "rsbbuffer")
SET(RSBBUFFER_BIN_NAME "rsb-buffer")

# --- global compiler flags ---

INCLUDE(PedanticCompilerWarnings)

SET(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib:\$ORIGIN/")

# --- dependency handling ---

FIND_PACKAGE(RSB 0.6 REQUIRED)
INCLUDE_DIRECTORIES(BEFORE SYSTEM ${RSB_INCLUDE_DIRS})

# --- source code ---

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(examples)
ADD_SUBDIRECTORY(3rdparty)

IF(BUILD_TESTS)
    ENABLE_TESTING()
    ADD_SUBDIRECTORY(test)
ENDIF()

# --- documentation generation ---

GENERATE_DOXYGEN(VERSION "${RSBBUFFER_VERSION}")

# --- coverage ---

#ENABLE_COVERAGE_REPORT(TARGETS ${RSBBUFFER_LIB_NAME} TESTS ${RSC_TEST_NAME} FILTER "*3rdparty*" "*test/*")

# --- sloccount ---

ENABLE_SLOCCOUNT(FOLDERS src)

# --- cppcheck ---

GENERATE_CPPCHECK(SOURCES src test examples
                          "${CMAKE_CURRENT_BINARY_DIR}/src" "${CMAKE_CURRENT_BINARY_DIR}/test" "${CMAKE_CURRENT_BINARY_DIR}/examples"
                  ENABLE_IDS style
                  INLINE_SUPPRESSION)
