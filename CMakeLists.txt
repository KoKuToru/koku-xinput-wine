cmake_minimum_required(VERSION 2.8)

project(koku-xinput-wine)
ADD_LIBRARY(koku-xinput-wine main.cpp config.cpp xinput.cpp)
target_link_libraries(koku-xinput-wine SDL)
set(CMAKE_CXX_FLAGS "-std=c++0x")
