cmake_minimum_required(VERSION 2.6)

#project(Socket)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib)

add_definitions(-DLINUX)
add_definitions(-DOPENSSL)

include_directories(Socket)

add_subdirectory(Socket)
add_subdirectory(SocketTest)

enable_testing ()

# test if the test INI file exist, otherwise default it to the one in SocketTest folder
IF (NOT TEST_INI_FILE)
    SET(TEST_INI_FILE "./SocketTest/template_test_conf.ini")
    MESSAGE(WARNING "You didn't provide an INI test configuration file.\
    Defaulting TEST_INI_FILE to ./SocketTest/template_test_conf.ini")
ENDIF()

add_test (NAME MyTest COMMAND test_socket ${TEST_INI_FILE})
