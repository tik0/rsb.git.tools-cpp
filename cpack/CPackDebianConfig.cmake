# To create a proper Debian/Ubuntu package, the following CMake
# options should be used:

#SET(CMAKE_BUILD_TYPE Release)
SET(CPACK_STRIP_FILES "TRUE")

# Operating system checks

IF(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    MESSAGE(FATAL_ERROR "Cannot configure CPack to generate Debian packages on non-linux systems.")
ENDIF()

INCLUDE(CheckLSBTypes)
IF((NOT LSB_DISTRIBUTOR_ID STREQUAL "ubuntu") AND (NOT LSB_DISTRIBUTOR_ID STREQUAL "debian"))
    MESSAGE(FATAL_ERROR "Cannot configure CPack to generate Debian packages on something that is not Ubuntu or Debian.")
ENDIF()

# Actual packaging options

SET(PACKAGE_BASE_NAME     "rsb-tools-${BINARY_SUFFIX}")
SET(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

SET(CPACK_PACKAGE_FILE_NAME     "${PACKAGE_BASE_NAME}-${CPACK_PACKAGE_VERSION}_${LSB_CODENAME}_${LSB_PROCESSOR_ARCH}")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING.txt")

# Generate postinst and prerm hooks
SET(POSTINST_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/postinst")
SET(PRERM_SCRIPT    "${CMAKE_CURRENT_BINARY_DIR}/prerm")
FILE(WRITE "${POSTINST_SCRIPT}" "#!/bin/sh\n\nset -e\n")
FILE(WRITE "${PRERM_SCRIPT}"    "#!/bin/sh\n\nset -e\n")
FOREACH(NAME "logger" "timesync")
FILE(APPEND "${POSTINST_SCRIPT}"
            "update-alternatives --install                      \\
               /usr/bin/${BINARY_PREFIX}${NAME}                 \\
               ${BINARY_PREFIX}${NAME}                          \\
               /usr/bin/${BINARY_PREFIX}${NAME}${BINARY_SUFFIX} \\
               80\n\n")
FILE(APPEND "${PRERM_SCRIPT}"
            "update-alternatives --remove                           \\
               ${BINARY_PREFIX}${NAME}                              \\
               /usr/bin/${BINARY_PREFIX}${NAME}${BINARY_SUFFIX}\n\n")
ENDFOREACH()
EXECUTE_PROCESS(COMMAND "chmod +x ${POSTINST_SCRIPT} ${PRERM_SCRIPT}")

SET(CPACK_GENERATOR                  "DEB")
SET(CPACK_DEBIAN_PACKAGE_NAME        "${PACKAGE_BASE_NAME}")
SET(CPACK_DEBIAN_PACKAGE_VERSION     "${CPACK_PACKAGE_VERSION}")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER  "Jan Moringen <jmoringe@techfak.uni-bielefeld.de>")
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Tools for the Robotics Service Bus (C++ implementation)
 Currently consists of
  * logger: console program for displaying RSB events in real-time
  * timesync: program for temporal synchronization of RSB events")
SET(CPACK_DEBIAN_PACKAGE_PRIORITY    "optional")
SET(CPACK_DEBIAN_PACKAGE_SECTION     "devel")
SET(CPACK_DEBIAN_ARCHITECTURE        "${CMAKE_SYSTEM_PROCESSOR}")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS     "libc6, rsc${VERSION_SUFFIX}, rsb${VERSION_SUFFIX}")
SET(CPACK_DEBIAN_PACKAGE_RECOMMENDS  "spread (>= 4.0)")

SET(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${POSTINST_SCRIPT};${PRERM_SCRIPT}")

MESSAGE(STATUS "Debian Package: ${CPACK_DEBIAN_PACKAGE_NAME} (${CPACK_DEBIAN_PACKAGE_VERSION}) [${CPACK_PACKAGE_FILE_NAME}.deb]")
