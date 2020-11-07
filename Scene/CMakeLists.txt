cmake_minimum_required(VERSION 3.12)
project(Scene VERSION 0.0.1)

file(GLOB_RECURSE SCENE_HEADER "./Scene/*.hpp" "./interface/*.hpp" "./driver/*.hpp")
file(GLOB_RECURSE SCENE_SOURCE "./Scene/include.cpp" "./interface/*.cpp" "./driver/*.cpp")

enable_language(CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(Render ${SCENE_HEADER} ${SCENE_SOURCE})

target_include_directories(Render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/interface)

target_link_libraries(Scene PUBLIC Render)
target_link_libraries(Scene PUBLIC Math)
target_link_libraries(Scene PUBLIC Common)
target_link_libraries(Scene PUBLIC Optick)