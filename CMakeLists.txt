cmake_minimum_required(VERSION 3.29)
project(PyPHP C)

set(CMAKE_C_STANDARD 23)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

include(CTest)
enable_testing()

add_executable(pyhp_demo_server src/pyhp_demo_server.c)

add_library(tiny_http_server_lib src/tiny_http/tiny_http_server_lib.c src/tiny_http/tiny_http_server_lib.h)

add_executable(assert_tiny_http_server_lib test/assert_tiny_http_server_lib.c)
target_link_libraries(assert_tiny_http_server_lib tiny_http_server_lib)

add_test(test_tiny_http_server_lib assert_tiny_http_server_lib)
