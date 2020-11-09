cmake_minimum_required(VERSION 3.12)
project(SceneComponents VERSION 0.0.1)

file(GLOB_RECURSE SCENECOMPONENTS_HEADER "./SceneComponents/*.hpp")
file(GLOB_RECURSE SCENECOMPONENTS_SOURCE "./SceneComponents/*.cpp")

enable_language(CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(SceneComponents ${SCENECOMPONENTS_HEADER} ${SCENECOMPONENTS_SOURCE})

target_include_directories(SceneComponents PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(SceneComponents PUBLIC Scene)
