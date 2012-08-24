# -*- mode: cmake -*-

# Global CMake Settings

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)

# Global Project Settings

PROJECT("rsb-tools-cpp")

SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Debug CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
ENDIF()

# Dependencies

SET(Boost_USE_VERSION 1.38 CACHE INTERNAL "Boost Version to use")
SET(Boost_USE_MULTITHREADED ON)
SET(Boost_USE_STATIC_LIBS OFF)
ADD_DEFINITIONS(-DBOOST_ALL_DYN_LINK)

IF(BOOST_ROOT)
  SET(Boost_NO_SYSTEM_PATHS ON)
ENDIF()

# RSC dependencies
FIND_PACKAGE(RSC REQUIRED)
MESSAGE(STATUS "RSC Version: ${RSC_VERSION}")
MESSAGE(STATUS "RSC libraries: ${RSC_LIBRARIES}")
MESSAGE(STATUS "RSC runtime library dirs: ${RSC_RUNTIME_LIBRARY_DIRS}")
MESSAGE(STATUS "RSC CMake module path: ${RSC_CMAKE_MODULE_PATH}")

# Use CMake modules in additional locations.
# Has to be done after RSC configuration, since RSC updates CMake's
# module path.
SET(PROJECT_CMAKE_PATH ${CMAKE_INSTALL_PREFIX}/share/cmake/Modules)
SET(CMAKE_MODULE_PATH "${PROJECT_CMAKE_PATH}" "${CMAKE_MODULE_PATH}" "${RSC_CMAKE_MODULE_PATH}")
MESSAGE(STATUS "Using CMake module path: ${CMAKE_MODULE_PATH}")

# Provided by RSC.
INCLUDE(PedanticCompilerWarnings)

FIND_PACKAGE(ProtocolBuffers REQUIRED)
IF(NOT PROTOBUF_FOUND)
    MESSAGE(FATAL_ERROR "protocol buffers not found")
ENDIF()

# May depend on updated CMake module path.
FIND_PACKAGE(Boost ${Boost_USE_VERSION} REQUIRED thread system filesystem program_options)
IF(NOT RSC_INTERNAL_BOOST_UUID)
    FIND_PACKAGE(BoostUUID REQUIRED)
ENDIF()

# RSB dependencies
# May depend on updated CMake module path.
FIND_PACKAGE(RSB REQUIRED)
MESSAGE(STATUS "RSB Version: ${RSB_VERSION}")
MESSAGE(STATUS "RSB libraries: ${RSB_LIBRARIES}")
MESSAGE(STATUS "RSB runtime library dirs: ${RSB_RUNTIME_LIBRARY_DIRS}")

ADD_DEFINITIONS(${RSB_PROTOCOL_CFLAGS})

INCLUDE_DIRECTORIES(BEFORE SYSTEM ${Boost_INCLUDE_DIRS}
                                  ${BOOSTUUID_INCLUDE_DIRS}
                                  ${PROTOBUF_INCLUDE_DIRS}
                                  ${RSC_INCLUDE_DIRS}
                                  ${RSB_INCLUDE_DIRS}
                                  ${RST_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

# Tools in sub-directories
SET(RSB_TOOLS_VERSION_MAJOR ${RSB_VERSION_MAJOR})
SET(RSB_TOOLS_VERSION_MINOR ${RSB_VERSION_MINOR})
SET(RSB_TOOLS_VERSION_PATCH ${RSB_VERSION_PATCH})
SET(VERSION_SUFFIX "${RSB_TOOLS_VERSION_MAJOR}.${RSB_TOOLS_VERSION_MINOR}")
SET(BINARY_PREFIX  "rsb-") # used by install rule in subdirs
SET(BINARY_SUFFIX "cpp${VERSION_SUFFIX}")
ADD_SUBDIRECTORY(logger)
ADD_SUBDIRECTORY(timesync)

# Packaging

SET(CPACK_PACKAGE_VERSION_MAJOR ${RSB_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${RSB_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${RSB_VERSION_PATCH})

SET(CPACK_PACKAGE_VENDOR "CoR-Lab Bielefeld University")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING.txt")

INCLUDE(ProvideFlexibleCPack)
