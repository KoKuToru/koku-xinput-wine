PROJECT(koku-xinput-wine)
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

ADD_LIBRARY(koku-xinput-wine SHARED main.cpp xinput.cpp device.cpp)

# Pull in pkgconfig
include(FindPkgConfig)

# - Try to find SDL
# Once done this will define
#
# SDL_FOUND - system has libSDL
# SDL_INCLUDE_DIRS - the libSDL include directory
# SDL_LIBRARIES - The libSDL libraries
if(PKG_CONFIG_FOUND)
    pkg_check_modules (SDL REQUIRED sdl2)
else()
    find_package(SDL2 REQURIED)
    list(APPEND SDL_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
    list(APPEND SDL_LIBRARIES ${SDL2_LIBRARIES})
endif()

# Link SDL
target_link_libraries(koku-xinput-wine ${SDL_LIBRARY})
set_target_properties(koku-xinput-wine PROPERTIES PREFIX "")
