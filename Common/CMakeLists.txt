cmake_minimum_required(VERSION 3.12)
project(Common VERSION 0.0.1)

file(GLOB_RECURSE COMMON_HEADER "./Common/*.hpp")
file(GLOB_RECURSE COMMON_SOURCE "./Common/*.cpp")

enable_language(CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(Common ${COMMON_HEADER} ${COMMON_SOURCE})

target_include_directories(Common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Common PUBLIC SQLite)
#target_link_libraries(MyRender PUBLIC ZLib)