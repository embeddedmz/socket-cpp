cmake_minimum_required(VERSION 2.6)

project(Socket)

IF(NOT CMAKE_BUILD_TYPE MATCHES Coverage)

find_package(OpenSSL)

file(GLOB_RECURSE source_files ./*)
add_library(socket STATIC ${source_files})

ENDIF()