set(SRCS http.c storage.c)

add_library(shock ${SRCS})

if(COVERAGE)
    include(CodeCoverage)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
    SETUP_TARGET_FOR_COVERAGE(coverage test/http_test coverage)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wc++-compat")
endif()

target_include_directories(shock
                            PUBLIC
                                ${CMAKE_SOURCE_DIR}/include)

if(CPPCHECK)
    find_package(cppcheck REQUIRED)
    include(CppcheckTargets)
    set(CPPCHECK_TEMPLATE_ARG ${CPPCHECK_TEMPLATE_ARG} --error-exitcode=2 --suppress=missingIncludeSystem "--suppress=unusedFunction")
    add_cppcheck(shock STYLE POSSIBLE_ERROR FAIL_ON_WARNINGS)
endif()
