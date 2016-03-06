PROJECT(koku-xinput-cosetproxyblanket)
cmake_minimum_required(VERSION 2.8)

# Check for C++11
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# Check for 32bit version
option(BUILD_M32 "Build library in 32Bit mode" ON)
if(BUILD_M32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
endif()

ADD_LIBRARY(${PROJECT_NAME} SHARED main.cpp device.cpp)

# Link SDL
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
