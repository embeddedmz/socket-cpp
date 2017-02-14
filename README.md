# TCP client/server API for C++
[![MIT license](https://img.shields.io/badge/license-MIT-blue.svg)](http://opensource.org/licenses/MIT)


## About
This is a simple TCP server/client for C++. Under Windows, it wraps WinSock and under Linux it wraps 
the related socket API (BSD compatible).
It is meant to be a portable and easy-to-use API to create a TCP server or client.

Upcoming features : creating secure sockets with OpenSSL and using the sockets in an async way.

Compilation has been tested with:
- GCC 5.4.0 (GNU/Linux Ubuntu 16.04 LTS)
- Microsoft Visual Studio 2015 (Windows 10)

## Usage
Create an object and provide to its constructor a callable object (for log printing) having this signature :

```cpp
void(const std::string&)
```

This section will be completed soon....

## Thread Safety

Do not share ASocket objects across threads.

## Installation
You will need CMake to generate a makefile for the static library or to build the tests/code coverage 
program.

Also make sure you have libcurl and Google Test installed.

You can follow this script https://gist.github.com/fideloper/f72997d2e2c9fbe66459 to install libcurl.

This tutorial will help you installing properly Google Test on Ubuntu: https://www.eriksmistad.no/getting-started-with-google-test-on-ubuntu/

The CMake script located in the tree will produce a makefile for the creation of a static library,
whereas the one under SocketTest will produce the unit tests program.

To create a debug static library, change directory to the one containing the first CMakeLists.txt

```Shell
cmake . -DCMAKE_BUILD_TYPE:STRING=Debug
make
```

To create a release static library, just change "Debug" by "Release".

The library will be found under lib/[BUILD_TYPE]/libsocket.a

For the unit tests program, first build the static library and use the same build type when
building it :

```Shell
cd SocketTest
cmake . -DCMAKE_BUILD_TYPE=Debug     # or Release
make
```

To run it :
```Shell
./bin/[BUILD_TYPE]/test_socket
```

## Run Unit Tests

You can generate an XML file of test results by adding this argument when calling the test program

```Shell
./bin/[BUILD_TYPE]/test_socket --gtest_output="xml:./TestSocket.xml"
```

## Memory Leak Check

Visual Leak Detector has been used to check memory leaks with the Windows build (Visual Sutdio 2015)
You can download it here: https://vld.codeplex.com/

To perform a leak check with the Linux build, you can do so :

```Shell
valgrind --leak-check=full ./bin/Debug/test_socket
```

## Code Coverage

The code coverage build doesn't use the static library but compiles and uses directly the 
socket API in the test program.

by the location of your ini file and launch the code coverage :

```Shell
cd SocketTest
cmake . -DCMAKE_BUILD_TYPE=Coverage
make
make coverage_socket
```

If everything is OK, the results will be found under ./SocketTest/coverage/index.html

Under Visual Studio, you can simply use OpenCppCoverage (https://opencppcoverage.codeplex.com/)

## CppCheck Compliancy

The C++ code of the Socket C++ API classes is Cppcheck compliant.

## Contribute
All contributions are highly appreciated. This includes updating documentation, writing code and unit tests
to increase code coverage and enhance tools.

Try to preserve the existing coding style (Hungarian notation, indentation etc...).
