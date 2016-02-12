cmake_minimum_required(VERSION 2.8)

# Pull in pkgconfig
include(FindPkgConfig)

project(koku-xinput-wine)
ADD_LIBRARY(koku-xinput-wine SHARED main.cpp config.cpp xinput.cpp device.cpp)

# Find SDL:
pkg_search_module(SDL REQUIRED sdl1.2)
include_directories(${SDL_INCLUDE_DIRS})

# Link SDL
target_link_libraries(koku-xinput-wine ${SDL_LIBRARIES})

# Set CXX flags
set(CMAKE_CXX_FLAGS "-std=c++0x -m32")

#For SDL2:
#target_link_libraries(koku-xinput-wine SDL2)
#set(CMAKE_CXX_FLAGS "-std=c++0x -m32 -DUSE_SDL2")

set_target_properties(koku-xinput-wine PROPERTIES PREFIX "")
