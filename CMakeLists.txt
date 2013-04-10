cmake_minimum_required(VERSION 2.8)

project(koku-xinput-wine)
ADD_LIBRARY(koku-xinput-wine SHARED main.cpp config.cpp xinput.cpp device.cpp)
target_link_libraries(koku-xinput-wine SDL)
set(CMAKE_CXX_FLAGS "-std=c++0x -m32")
set_target_properties(koku-xinput-wine PROPERTIES PREFIX "")
