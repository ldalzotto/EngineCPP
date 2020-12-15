cmake_minimum_required(VERSION 3.12)
project(Middleware VERSION 0.0.1)

file(GLOB_RECURSE MIDDLEWARE_HEADER "./Middleware/*.hpp")
file(GLOB_RECURSE MIDDLEWARE_SOURCE "./Middleware/*.cpp")

enable_language(CXX)

add_library(Middleware INTERFACE)
target_include_directories(Middleware INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Middleware INTERFACE Common)
target_link_libraries(Middleware INTERFACE Render)
target_link_libraries(Middleware INTERFACE Scene)
target_link_libraries(Middleware INTERFACE SceneComponents)
target_link_libraries(Middleware INTERFACE SceneSerialization)
