OPTION(OPTION_BUILD_EXAMPLES "Whether to build the examples or not" TRUE)
OPTION(OPTION_BUILD_TESTS "Whether to build the tests or not" TRUE)

SET(LIBRARY_NAME "rsbts")
SET(BINARY_NAME "timesync")

# --- dependencies ---

ADD_SUBDIRECTORY(src)
IF(OPTION_BUILD_TESTS)
    INCLUDE(ProvideGoogleMock)
    IF(GMOCK_AVAILABLE)
        ADD_SUBDIRECTORY(test)
    ELSE()
        MESSAGE(WARNING "Could not build unit tests even though desired because Google Mock could not be installed.")
    ENDIF()
ENDIF()
ENABLE_TESTING()
IF(OPTION_BUILD_EXAMPLES) # AND SPREAD_EXECUTABLE)
    ADD_SUBDIRECTORY(examples)
ENDIF()
