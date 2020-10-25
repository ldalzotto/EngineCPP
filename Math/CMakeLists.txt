cmake_minimum_required(VERSION 3.12)
project(Math VERSION 0.0.1)

file(GLOB_RECURSE MATH_HEADER "./Math/*.hpp")
file(GLOB_RECURSE MATH_SOURCE "./Math/*.cpp")

enable_language(CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(Math ${MATH_HEADER} ${MATH_SOURCE})

target_include_directories(Math PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#target_link_libraries(MyRender PUBLIC ZLib)