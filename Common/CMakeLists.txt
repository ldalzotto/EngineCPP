cmake_minimum_required(VERSION 3.12)
project(Common VERSION 0.0.1)

enable_language(CXX)

add_library(Common INTERFACE)
target_include_directories(Common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Common INTERFACE SQLite)
