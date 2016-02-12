PROJECT(koku-xinput-wine)
cmake_minimum_required(VERSION 2.8)

# Pull in pkgconfig
include(FindPkgConfig)

project(koku-xinput-wine)
ADD_LIBRARY(koku-xinput-wine SHARED main.cpp config.cpp xinput.cpp device.cpp)

# - Try to find SDL
# Once done this will define
#
# SDL_FOUND - system has libSDL
# SDL_INCLUDE_DIRS - the libSDL include directory
# SDL_LIBRARIES - The libSDL libraries

if(PKG_CONFIG_FOUND)
  pkg_check_modules (SDL)
else()
  find_package(SDL)
  list(APPEND SDL_INCLUDE_DIRS ${SDL_INCLUDE_DIR})
  list(APPEND SDL_LIBRARIES ${SDL_LIBRARIES})
endif()

# Link SDL
target_link_libraries(koku-xinput-wine ${SDL_LIBRARY})

# Set CXX flags
set(CMAKE_CXX_FLAGS "-std=c++0x -m32")

#For SDL2:
#target_link_libraries(koku-xinput-wine SDL2)
#set(CMAKE_CXX_FLAGS "-std=c++0x -m32 -DUSE_SDL2")

set_target_properties(koku-xinput-wine PROPERTIES PREFIX "")
