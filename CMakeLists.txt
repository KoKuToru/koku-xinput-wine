cmake_minimum_required(VERSION 3.5)
project(koku-xinput-wine)

set(USING_CROSS_COMPILER OFF)
if (CMAKE_C_COMPILER MATCHES "i686-w64-mingw32-gcc" OR
    CMAKE_C_COMPILER MATCHES "x86_64-w64-mingw32-gcc")
  set(USING_CROSS_COMPILER ON)
endif()

set(USING_64BIT_COMPILER OFF)
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(USING_64BIT_COMPILER ON)
endif()

option(BUILD_LIBRARY "Build main library" ON)
option(BUILD_32BIT_LIBRARY "Build main library for 32-bit wine" ON)
option(BUILD_64BIT_LIBRARY "Build main library for 64-bit wine" ${USING_64BIT_COMPILER})
option(BUILD_TESTS "Build test executables" ON)

if (BUILD_LIBRARY)
  if (USING_CROSS_COMPILER)
    message(WARNING "Called with cross-compiler; skipping main library")
  elseif (NOT BUILD_32BIT_LIBRARY AND NOT BUILD_64BIT_LIBRARY)
    message(WARNING "No architectures enabled; skipping main library")
  else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)

    find_path(WINE_INCLUDE_DIR windows.h
      HINTS ${WINEROOT}/include ${WINE_ROOT}/include ${WINE_INCLUDEDIR}
      PATHS /opt/wine-staging/include
      PATH_SUFFIXES wine-development/windows wine/windows)
    if (WINE_INCLUDE_DIR)
      message(STATUS "Found wine headers: ${WINE_INCLUDE_DIR}")
    else()
      message(FATAL_ERROR "Couldn't find wine headers!")
    endif()

    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_SHARED_LIBRARY_PREFIX "")

    set(KOKU_SOURCE_FILES main.cpp xinput.cpp device.cpp)
    set(KOKU_INCLUDE_DIRS ${WINE_INCLUDE_DIR} ${SDL2_INCLUDE_DIR})
    set(KOKU_LINK_LIBRARIES ${SDL2_LIBRARIES})
    set(KOKU_COMPILE_OPTIONS -Wall -Wextra
      -Wno-attributes -Wno-ignored-attributes -Wno-subobject-linkage
      -Wno-unused-parameter -Wno-unused-variable)

    if (BUILD_32BIT_LIBRARY)
      add_library(koku-xinput-wine SHARED ${KOKU_SOURCE_FILES})
      target_include_directories(koku-xinput-wine PRIVATE ${KOKU_INCLUDE_DIRS})
      target_link_libraries(koku-xinput-wine PRIVATE -m32 ${KOKU_LINK_LIBRARIES})
      target_compile_options(koku-xinput-wine PRIVATE -m32 ${KOKU_COMPILE_OPTIONS})
    endif()

    if (BUILD_64BIT_LIBRARY)
      add_library(koku-xinput-wine64 SHARED ${KOKU_SOURCE_FILES})
      target_include_directories(koku-xinput-wine64 PRIVATE ${KOKU_INCLUDE_DIRS})
      target_link_libraries(koku-xinput-wine64 PRIVATE ${KOKU_LINK_LIBRARIES})
      target_compile_options(koku-xinput-wine64 PRIVATE ${KOKU_COMPILE_OPTIONS})
    endif()
  endif()
endif()

if (BUILD_TESTS)
  if (USING_CROSS_COMPILER)
    add_executable(ditest test/ditest.cpp)
    target_link_libraries(ditest PRIVATE -static -static-libgcc
      dinput dinput8 dxguid user32 ole32 oleaut32)
    add_executable(xitest test/xitest.cpp)
    target_link_libraries(xitest PRIVATE -static -static-libgcc
      xinput)
  else()
    find_program(I686_MINGW64_GCC i686-w64-mingw32-gcc)
    if (I686_MINGW64_GCC)
      execute_process(
        COMMAND ${CMAKE_COMMAND}
          -B${CMAKE_BINARY_DIR}/i686-w64-mingw32
          -H${CMAKE_SOURCE_DIR}
          -DCMAKE_TOOLCHAIN_FILE=${CMAKE_SOURCE_DIR}/Toolchain-Windows-i686.cmake)
      add_custom_target(i686-w64-mingw32-tests ALL
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/i686-w64-mingw32)
    else()
      message(WARNING "No 32-bit cross-compiler; skipping 32-bit tests")
    endif()

    find_program(X86_64_MINGW64_GCC x86_64-w64-mingw32-gcc)
    if (X86_64_MINGW64_GCC)
      execute_process(
        COMMAND ${CMAKE_COMMAND}
          -B${CMAKE_BINARY_DIR}/x86_64-w64-mingw32
          -H${CMAKE_SOURCE_DIR}
          -DCMAKE_TOOLCHAIN_FILE=${CMAKE_SOURCE_DIR}/Toolchain-Windows-x86_64.cmake)
      add_custom_target(x86_64-w64-mingw32-tests ALL
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/x86_64-w64-mingw32)
    else()
      message(WARNING "No 64-bit cross-compiler; skipping 64-bit tests")
    endif()
  endif()
endif()
