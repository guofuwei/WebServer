cmake_minimum_required(VERSION 3.16)
project(WebServer)

set(CMAKE_CXX_STANDARD 17)
find_package(Threads REQUIRED)

include_directories(.)
include_directories(include)

aux_source_directory(source DIR_SRCS)

add_executable(WebServer ${DIR_SRCS}  main.cpp)
target_link_libraries(WebServer Threads::Threads)
