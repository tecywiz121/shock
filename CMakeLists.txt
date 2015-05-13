cmake_minimum_required(VERSION 2.8.12)
project(shock)

if(BUILD_SHARED_LIBS)
    add_definitions(-DSHOCK_SHARED)
endif()

option(CPPCHECK "enable scanning the source code with cppcheck" ON)
option(COVERAGE "enable code coverage flags" OFF)
option(ARDUINO "build arduino library" OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -Wall -Wextra -pedantic -Werror -std=c11")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -Wall -Wextra -pedantic -Werror -std=c++11")
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
endif()

if(COVERAGE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

add_subdirectory(src)

enable_testing()
add_subdirectory(test)

add_subdirectory(ex)

if(ARDUINO)
    add_subdirectory(arduino)
endif()
