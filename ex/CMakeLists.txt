add_executable(shocked main)
target_link_libraries(shocked shock)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter -Wno-unused-function")
