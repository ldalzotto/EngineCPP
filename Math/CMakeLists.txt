cmake_minimum_required(VERSION 3.12)
project(Math VERSION 0.0.1)

enable_language(CXX)

add_library(Math INTERFACE)
target_include_directories(Math INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})


add_executable(MathTest "./test/test.cpp" "Math/geometry_def.hpp")
target_include_directories(MathTest INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(MathTest PUBLIC Math)