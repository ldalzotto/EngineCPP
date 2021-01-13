cmake_minimum_required(VERSION 3.12)
project(Math2 VERSION 0.0.1)

enable_language(CXX)

add_library(Math2 INTERFACE)
target_include_directories(Math2 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(Math2Test "./test/test.cpp")
target_include_directories(Math2Test INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Math2Test PUBLIC Common2)
target_link_libraries(Math2Test PUBLIC Math2)