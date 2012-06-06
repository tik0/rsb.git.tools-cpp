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

ENABLE_TESTING()
IF(BUILD_TESTS)
    INCLUDE(ProvideGoogleMock)
    IF(GMOCK_AVAILABLE)
        ADD_SUBDIRECTORY(test)
    ELSE()
        MESSAGE(WARNING "Could not build unit tests even though desired because Google Mock could not be installed.")
    ENDIF()
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
