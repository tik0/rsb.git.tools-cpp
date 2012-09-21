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
FIND_PACKAGE(RSB REQUIRED)
MESSAGE(STATUS "RSB Version: ${RSB_VERSION}")
MESSAGE(STATUS "RSB libraries: ${RSB_LIBRARIES}")
MESSAGE(STATUS "RSB runtime library dirs: ${RSB_RUNTIME_LIBRARY_DIRS}")

LIST(INSERT CMAKE_MODULE_PATH 0 ${RSC_CMAKE_MODULE_PATH})

# Provided by RSC.
INCLUDE(PedanticCompilerWarnings)
INCLUDE(GenerateDoxygen)
INCLUDE(EnableSlocCount)
INCLUDE(GenerateDoxygen)
INCLUDE(GenerateCppcheck)
INCLUDE(DefineProjectVersion)
INCLUDE(CurrentDate)

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
SET(BINARY_PREFIX  "rsb-") # used by install rule in subdirs
SET(BINARY_SUFFIX "cpp${VERSION_SUFFIX}")

ADD_SUBDIRECTORY(buffers)
ADD_SUBDIRECTORY(logger)
ADD_SUBDIRECTORY(timesync)

# Packaging

SET(CPACK_PACKAGE_VERSION_MAJOR ${RSB_TOOLS_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${RSB_TOOLS_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${RSB_TOOLS_VERSION_PATCH})

SET(CPACK_PACKAGE_VENDOR "CoR-Lab Bielefeld University")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING.txt")

INCLUDE(ProvideFlexibleCPack)
