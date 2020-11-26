cmake_minimum_required(VERSION 3.12)
project(Input VERSION 0.0.1)

file(GLOB_RECURSE INPUT_HEADER "./Input/*.hpp" "./interface/*.hpp")
file(GLOB_RECURSE INPUT_SOURCE "./Input/*.cpp" "./interface/*.cpp")

enable_language(CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(Input ${INPUT_HEADER} ${INPUT_SOURCE})

target_include_directories(Input PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/interface)

target_link_libraries(Input PUBLIC Common)
target_link_libraries(Input PUBLIC GLFW)