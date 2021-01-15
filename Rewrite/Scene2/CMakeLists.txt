cmake_minimum_required(VERSION 3.12)
project(Scene2 VERSION 0.0.1)

enable_language(CXX)

add_library(Scene2 INTERFACE)
target_include_directories(Scene2 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Scene2 INTERFACE Common2)
target_link_libraries(Scene2 INTERFACE Math2)

add_executable(Scene2Test "./test/test.cpp" "Scene2/component2.hpp")
target_include_directories(Scene2Test INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Scene2Test PUBLIC Scene2)