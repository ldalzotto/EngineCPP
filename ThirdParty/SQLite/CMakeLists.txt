cmake_minimum_required(VERSION 3.12)
project(SQLite VERSION 0.0.1)

file(GLOB_RECURSE SQLite_HEADER "./public/*.h")

add_library(SQLite INTERFACE)

target_include_directories(SQLite INTERFACE "public/")
target_link_libraries(SQLite INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/lib/SQLite.lib")
