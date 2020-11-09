cmake_minimum_required(VERSION 3.12)
project(Middleware VERSION 0.0.1)

file(GLOB_RECURSE MIDDLEWARE_HEADER "./Middleware/*.hpp")
file(GLOB_RECURSE MIDDLEWARE_SOURCE "./Middleware/*.cpp")

enable_language(CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(Middleware ${MIDDLEWARE_HEADER} ${MIDDLEWARE_SOURCE})

target_include_directories(Middleware PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Middleware PUBLIC Common)
target_link_libraries(Middleware PUBLIC Render)
target_link_libraries(Middleware PUBLIC Scene)
target_link_libraries(Middleware PUBLIC SceneComponents)
