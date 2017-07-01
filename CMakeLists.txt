cmake_minimum_required(VERSION 3.6)
project(koku-xinput-wine)

if (NOT CMAKE_C_COMPILER MATCHES "i686-w64-mingw32-gcc"
    AND NOT CMAKE_C_COMPILER MATCHES "x86_64-w64-mingw32-gcc")
  find_program(X86_64_MINGW64_GCC x86_64-w64-mingw32-gcc)
  if (X86_64_MINGW64_GCC)
    execute_process(
      COMMAND ${CMAKE_COMMAND}
        -B${CMAKE_BINARY_DIR}/x86_64-w64-mingw32
        -H${CMAKE_SOURCE_DIR}
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_SOURCE_DIR}/Toolchain-Windows-x86_64.cmake)
    add_custom_target(x86_64-w64-mingw32-tests ALL
      COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/x86_64-w64-mingw32)
  endif()

  find_program(I686_MINGW64_GCC i686-w64-mingw32-gcc)
  if (I686_MINGW64_GCC)
    execute_process(
      COMMAND ${CMAKE_COMMAND}
        -B${CMAKE_BINARY_DIR}/i686-w64-mingw32
        -H${CMAKE_SOURCE_DIR}
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_SOURCE_DIR}/Toolchain-Windows-i686.cmake)
    add_custom_target(i686-w64-mingw32-tests ALL
      COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/i686-w64-mingw32)
  endif()

  option(BUILD_M32 "Build library in 32Bit mode" ON)
  if(BUILD_M32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
    add_compile_options(-m32)
  endif()

  add_library(koku-xinput-wine SHARED main.h main.cpp xinput.h xinput.cpp
    device.h device.cpp)
  set_target_properties(koku-xinput-wine PROPERTIES CXX_STANDARD 11 PREFIX "")
  target_include_directories(koku-xinput-wine PRIVATE
     /usr/include/wine-development/windows
     /usr/include/wine/windows)

  find_package(PkgConfig REQUIRED)
  pkg_check_modules(SDL2 REQUIRED sdl2)
  target_include_directories(koku-xinput-wine PRIVATE ${SDL2_INCLUDE_DIR})
  target_link_libraries(koku-xinput-wine PRIVATE ${SDL2_LIBRARIES})
else()
  add_executable(ditest test/ditest.cpp)
  target_link_libraries(ditest PRIVATE -static -static-libgcc
    dinput dinput8 dxguid user32 ole32 oleaut32)
  add_executable(xitest test/xitest.cpp)
  target_link_libraries(xitest PRIVATE -static -static-libgcc
    xinput)
endif()
