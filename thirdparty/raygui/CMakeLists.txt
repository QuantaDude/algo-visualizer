cmake_minimum_required(VERSION 3.21)
project(raygui)

add_library(raygui)
target_sources(raygui PRIVATE "src/raygui.c")
target_include_directories(raygui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(raygui PUBLIC raylib_static)

