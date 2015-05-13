set(LIBSHOCK_SOURCE_DIR "${shock_SOURCE_DIR}/src")
set(LIBSHOCK_INCLUDE_DIR "${shock_SOURCE_DIR}/include")

install(DIRECTORY
            "${CURRENT_SOURCE_DIR}"
        DESTINATION
            "Shock"
        PATTERN ".*" EXCLUDE
        PATTERN "CMakeLists.txt" EXCLUDE)

install(FILES
            "${shock_SOURCE_DIR}/LICENSE"
            "${LIBSHOCK_SOURCE_DIR}/http.c"
            "${LIBSHOCK_SOURCE_DIR}/http_pvt.h"
        DESTINATION
            "Shock")

install(DIRECTORY
            "${LIBSHOCK_INCLUDE_DIR}/shock"
        DESTINATION
            "Shock"
        FILES_MATCHING PATTERN
            "*.h")

set(CPACK_GENERATOR "ZIP")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

include(CPack)
