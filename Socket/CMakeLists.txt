cmake_minimum_required(VERSION 2.6)

project(Socket)

IF(MSVC OR NOT CMAKE_BUILD_TYPE MATCHES Coverage)

if(NOT SOCKET_CPP_BUILD_WITHOUT_SECURE_CLASSES)
	IF(NOT MSVC)
	find_package(OpenSSL)
	ELSE()
	find_package(OpenSSL REQUIRED)
	include_directories("${OPENSSL_INCLUDE_DIR}")
	ENDIF()
endif()

file(GLOB_RECURSE source_files ./*)
add_library(socket STATIC ${source_files})

ENDIF()