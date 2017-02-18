# TCP client/server API for C++
[![MIT license](https://img.shields.io/badge/license-MIT-blue.svg)](http://opensource.org/licenses/MIT)


## About
This is a simple TCP server/client for C++. Under Windows, it wraps WinSock and under Linux it wraps 
the related socket API (BSD compatible). It wraps also OpenSSL to create secure client/server sockets.

It is meant to be a portable and easy-to-use API to create a TCP server or client with or without SSL/TLS
support.

Upcoming features : using the sockets in an async way and proxy support.

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

To run it, you must indicate the path of the INI conf file (see the section below)
```Shell
./bin/[BUILD_TYPE]/test_socket /path_to_your_ini_file/conf.ini
```

Compile both the library and test program with the macro OPENSSL to use the SSL/TLS secured classes.

## Run Unit Tests

[simpleini](https://github.com/brofield/simpleini) is used to gather unit tests parameters from
an INI configuration file. You need to fill that file with some parameters.
You can also disable some tests (HTTP proxy for instance) and indicate
parameters only for the enabled tests. A template of the INI file already exists under TestHTTP/

e.g. to enable SSL/TLS tests :

```ini
[tests]
tcp-ssl=yes

[tcp-ssl]
server_port=4242
ca_file=CAfile.pem
ssl_cert_file=site.cert
ssl_key_file=privkey.pem
```

You can also generate an XML file of test results by adding this argument when calling the test program

```Shell
./bin/[BUILD_TYPE]/test_socket /path_to_your_ini_file/conf.ini --gtest_output="xml:./TestSocket.xml"
```

## Memory Leak Check

Visual Leak Detector has been used to check memory leaks with the Windows build (Visual Sutdio 2015)
You can download it here: https://vld.codeplex.com/

To perform a leak check with the Linux build, you can do so :

```Shell
valgrind --leak-check=full ./bin/Debug/test_socket /path_to_ini_file/conf.ini
```

## Code Coverage

The code coverage build doesn't use the static library but compiles and uses directly the 
socket API in the test program.

First of all, in TestHTTP/CMakeLists.txt, find and repalce :
```
"/home/amzoughi/Test/http_github.ini"
```
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
