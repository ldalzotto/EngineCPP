cmake_minimum_required(VERSION 3.12)
project(SceneComponents VERSION 0.0.1)

enable_language(CXX)

add_library(SceneComponents INTERFACE)
target_include_directories(SceneComponents INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(SceneComponents INTERFACE Scene)
target_link_libraries(SceneComponents INTERFACE Render)
