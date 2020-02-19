cmake_minimum_required(VERSION 3.10)

project(catchpoint_assignament)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS  "-g -Wall -Wextra -pedantic")

#dependencies
include(FindPkgConfig)
pkg_check_modules(CURLPP REQUIRED curlpp)
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(CURL REQUIRED)
find_package(Threads REQUIRED)

add_executable(rpa main.cpp)


target_link_libraries(rpa PRIVATE nlohmann_json::nlohmann_json ${CMAKE_THREAD_LIBS_INIT} curl curlpp)


