cmake_minimum_required(VERSION 3.12)
project(Common VERSION 0.0.1)

enable_language(CXX)

add_library(Common2 INTERFACE)
target_include_directories(Common2 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
# target_link_libraries(Common INTERFACE SQLite)

add_executable(Common2Test "./test/test.cpp")
target_include_directories(Common2Test INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Common2Test PUBLIC Common2)