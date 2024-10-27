cmake_minimum_required(VERSION 3.29)
project(PyPHP C)

set(CMAKE_C_STANDARD 23)

add_compile_options(-fsanitize=address)
add_compile_options(-fno-omit-frame-pointer)
add_compile_options(-fsanitize-address-use-after-scope)
add_compile_options(-fsanitize-address-outline-instrumentation)
add_link_options(-fsanitize=address)
add_link_options(-fno-omit-frame-pointer)
add_link_options(-fsanitize-address-use-after-scope)
add_link_options(-fsanitize-address-outline-instrumentation)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

include(CTest)
enable_testing()

add_executable(pyhp_demo_server src/pyhp_demo_server.c)
