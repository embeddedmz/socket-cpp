
IF(MSVC OR NOT CMAKE_BUILD_TYPE MATCHES Coverage)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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

target_include_directories(socket PRIVATE ${OPENSSL_INCLUDE_DIR})

install(TARGETS socket)

ENDIF()
