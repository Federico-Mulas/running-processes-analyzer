cmake_minimum_required(VERSION 3.10)

project(catchpoint_assignament)

find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(CURL REQUIRED)
find_package(Threads REQUIRED)
#find_package(curlpp 0.8.1 REQUIRED)


add_executable(cpa main.cpp)


target_link_libraries(cpa PRIVATE nlohmann_json::nlohmann_json ${CMAKE_THREAD_LIBS_INIT} curl curlpp)


