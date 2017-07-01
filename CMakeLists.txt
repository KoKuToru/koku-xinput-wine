cmake_minimum_required(VERSION 3.6)
project(koku-xinput-wine)

option(BUILD_M32 "Build library in 32Bit mode" ON)
if(BUILD_M32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
  add_compile_options(-m32)
endif()

add_library(koku-xinput-wine SHARED main.h main.cpp xinput.h xinput.cpp
  device.h device.cpp)
set_target_properties(koku-xinput-wine PROPERTIES CXX_STANDARD 11 PREFIX "")

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
target_include_directories(koku-xinput-wine PRIVATE ${SDL2_INCLUDE_DIR})
target_link_libraries(koku-xinput-wine PRIVATE ${SDL2_LIBRARIES})
