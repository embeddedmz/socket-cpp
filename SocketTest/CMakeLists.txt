# Code coverage setup
IF(CMAKE_BUILD_TYPE MATCHES Coverage)
   INCLUDE(CodeCoverage.cmake)
   SET(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
   SET(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
ENDIF(CMAKE_BUILD_TYPE MATCHES Coverage)

# Locate OpenSSL
if(NOT SOCKET_CPP_BUILD_WITHOUT_SECURE_CLASSES)
	if(NOT MSVC)
		find_package(OpenSSL)
	else()
		find_package(OpenSSL REQUIRED)
		include_directories("${OPENSSL_INCLUDE_DIR}")
	endif()
endif()

# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS}) # useless but test before removing it

include_directories(../Socket)
include_directories(./simpleini)
include_directories(./)

IF(NOT MSVC AND CMAKE_BUILD_TYPE MATCHES Coverage)

file(GLOB_RECURSE socket_source_files ../Socket/*)

#Output Setup
add_executable(test_socket Tests.cpp test_utils.cpp ${socket_source_files})

target_include_directories(test_socket PRIVATE ${OPENSSL_INCLUDE_DIR})

#Link setup
target_link_libraries(test_socket ${GTEST_LIBRARIES} pthread ${OPENSSL_LIBRARIES})

SETUP_TARGET_FOR_COVERAGE(
           coverage_socket      # Name for custom target.
           test_socket          # Name of the test driver executable that runs the tests.
                                # NOTE! This should always have a ZERO as exit code
                                # otherwise the coverage generation will not complete.
           coverage             # Name of output directory.
           ${COVERAGE_INI_FILE} # Optional fourth parameter is passed as arguments to _testrunner
                                # Pass them in list form, e.g.: "-j;2" for -j 2
           )
ELSE()

link_directories(${CMAKE_BINARY_DIR}/lib)

#Output Setup
add_executable(test_socket Tests.cpp test_utils.cpp)

target_include_directories(test_socket PRIVATE ${OPENSSL_INCLUDE_DIR})

#Link setup
if(NOT MSVC)
	target_link_libraries(test_socket socket ${GTEST_LIBRARIES} pthread ${OPENSSL_LIBRARIES})
else()
	target_link_libraries(test_socket socket ${GTEST_LIBRARIES} ${OPENSSL_LIBRARIES})
endif()

ENDIF()

