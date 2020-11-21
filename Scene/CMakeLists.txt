cmake_minimum_required(VERSION 3.12)
project(Scene VERSION 0.0.1)

file(GLOB_RECURSE SCENE_HEADER "./Scene/*.hpp" "./interface/*.hpp")
file(GLOB_RECURSE SCENE_SOURCE "./Scene/*.cpp" "./Scene/include.cpp" "./interface/*.cpp")

enable_language(CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(Scene ${SCENE_HEADER} ${SCENE_SOURCE})

target_include_directories(Scene PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/interface)

target_link_libraries(Scene PUBLIC Math)
target_link_libraries(Scene PUBLIC Common)
target_link_libraries(Scene PUBLIC Optick)


add_subdirectory(components)

project(SceneTest VERSION 0.0.1)

add_executable(SceneTest ${SCENE_HEADER} "./test/test.cpp")

target_include_directories(SceneTest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/interface)

target_link_libraries(SceneTest PUBLIC Math)
target_link_libraries(SceneTest PUBLIC Common)
target_link_libraries(SceneTest PUBLIC Optick)
target_link_libraries(SceneTest PUBLIC SceneComponents)