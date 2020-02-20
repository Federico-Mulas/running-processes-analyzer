cmake_minimum_required(VERSION 3.10)

project(catchpoint_assignament)

message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

IF(CMAKE_BUILD_TYPE MATCHES Debug)
  message("Debug build.")
  set(CMAKE_CXX_FLAGS  "-g -Wall -Wextra -pedantic")
ELSEIF(CMAKE_BUILD_TYPE MATCHES Release)
  message("Release build.")
  set(CMAKE_CXX_FLAGS  "-O3")
ELSE()
  message("Some other build type.")
ENDIF()


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#dependencies
include(FindPkgConfig)
pkg_check_modules(CURLPP REQUIRED curlpp)
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(CURL REQUIRED)
find_package(Threads REQUIRED)
find_package(gflags REQUIRED)

add_executable(rpa main.cpp parser.cpp)




target_link_libraries(rpa PRIVATE nlohmann_json::nlohmann_json ${CMAKE_THREAD_LIBS_INIT} curl curlpp gflags stdc++fs)


