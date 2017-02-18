cmake_minimum_required(VERSION 2.6)

# Project configuration
project(Socket)
set(LIBRARY_OUTPUT_PATH lib/${CMAKE_BUILD_TYPE})
set(CMAKE_CXX_STANDARD 14)

add_definitions(-DLINUX)
add_definitions(-DOPENSSL)

include_directories(Socket)

file(GLOB_RECURSE source_files Socket/*)
add_library(socket STATIC ${source_files})
