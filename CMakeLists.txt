# -*- mode: cmake -*-

# Global CMake Settings

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)

# Global Project Settings

PROJECT("rsb-tools-cpp")

SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

OPTION(OPTION_BUILD_EXAMPLES "Whether to build the examples or not" TRUE)
OPTION(OPTION_BUILD_TESTS "Whether to build the tests or not" TRUE)

# default version information
SET(VERSION_MAJOR   "0" CACHE STRING "Major project version part")
SET(VERSION_MINOR   "8" CACHE STRING "Minor project version part")
SET(RSB_TOOLS_ABI_VERSION "2")

# Dependencies

# RSB dependencies
# May depend on updated CMake module path.
FIND_PACKAGE(RSB "${VERSION_MAJOR}.${VERSION_MINOR}" REQUIRED)

LIST(INSERT CMAKE_MODULE_PATH 0 ${RSC_CMAKE_MODULE_PATH})

# Provided by RSC.
INCLUDE(PedanticCompilerWarnings)
INCLUDE(GenerateDoxygen)
INCLUDE(EnableSlocCount)
INCLUDE(GenerateDoxygen)
INCLUDE(GenerateCppcheck)
INCLUDE(DefineProjectVersion)
INCLUDE(CurrentDate)
INCLUDE(InstallFilesRecursive)

# finally we can deduce the real project version with the RSC macros
DEFINE_PROJECT_VERSION(RSB_TOOLS_ ${VERSION_MAJOR} ${VERSION_MINOR} 0 "archive")
CURRENT_DATE(RSB_TOOLS_BUILD_DATE)
SET(VERSION_SUFFIX "${RSB_TOOLS_VERSION_MAJOR}.${RSB_TOOLS_VERSION_MINOR}")
SET(SO_VERSION     "${RSB_TOOLS_VERSION_MAJOR}.${RSB_TOOLS_VERSION_MINOR}")

ADD_DEFINITIONS(${RSB_PROTOCOL_CFLAGS})

INCLUDE_DIRECTORIES(BEFORE SYSTEM ${RSC_INCLUDE_DIRS}
                                  ${RSB_INCLUDE_DIRS}
                                  ${RST_INCLUDE_DIRS})

IF(OPTION_BUILD_TESTS)
    ENABLE_TESTING()
    INCLUDE(ProvideGoogleMock)
    IF(NOT GMOCK_AVAILABLE)
        MESSAGE(WARNING "Could not build unit tests even though desired because Google Mock could not be installed.")
    ENDIF()
ENDIF()

# Tools in sub-directories

SET(BUFFER_LIBRARY_NAME rsbbuffer)
SET(BUFFER_BINARY_NAME buffer)

SET(LOGGER_BINARY_NAME "logger")

SET(TIMESYNC_LIBRARY_NAME "rsbts")
SET(TIMESYNC_BINARY_NAME "timesync")

SET(BINARY_PREFIX  "rsb-") # used by install rule in subdirs
SET(BINARY_SUFFIX "cpp${VERSION_SUFFIX}")

ADD_SUBDIRECTORY(src)
IF(OPTION_BUILD_EXAMPLES)
    ADD_SUBDIRECTORY(examples)
ENDIF()
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

# Packaging

SET(CPACK_PACKAGE_VERSION_MAJOR ${RSB_TOOLS_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${RSB_TOOLS_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${RSB_TOOLS_VERSION_PATCH})

SET(CPACK_PACKAGE_VENDOR "CoR-Lab Bielefeld University")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING.txt")

INCLUDE(ProvideFlexibleCPack)
