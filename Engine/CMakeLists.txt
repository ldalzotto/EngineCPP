cmake_minimum_required(VERSION 3.12)
project(Engine VERSION 0.0.1)

file(GLOB_RECURSE RENDER_HEADER "./Engine/*.hpp" "./interface/*.hpp" "./driver/*.hpp")
file(GLOB_RECURSE RENDER_SOURCE "./Engine/*.cpp" "./interface/*.cpp" "./driver/*.cpp")

enable_language(CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(Engine ${RENDER_HEADER} ${RENDER_SOURCE})

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/interface)

target_link_libraries(Engine PUBLIC GLFW)
target_link_libraries(Engine PUBLIC Render)