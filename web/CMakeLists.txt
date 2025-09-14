cmake_minimum_required(VERSION 4.0)
project(web)
set(CMAKE_CXX_STANDARD 20)

if(EMSCRIPTEN)
      include_directories(
        ${CMAKE_SYSROOT}/include
        ${CMAKE_SYSROOT}/include/emscripten
    )

   add_library(web STATIC src/web.cpp)
  target_include_directories(web PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
endif()

