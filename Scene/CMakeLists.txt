cmake_minimum_required(VERSION 3.12)
project(Scene VERSION 0.0.1)

enable_language(CXX)

add_library(Scene INTERFACE)
target_include_directories(Common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/interface)
target_link_libraries(Scene INTERFACE Math)
target_link_libraries(Scene INTERFACE Common)
target_link_libraries(Scene INTERFACE Optick)


add_subdirectory(components)

project(SceneTest VERSION 0.0.1)

add_executable(SceneTest ${SCENE_HEADER} "./test/test.cpp")

target_include_directories(SceneTest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/interface)

target_link_libraries(SceneTest PUBLIC Math)
target_link_libraries(SceneTest PUBLIC Common)
target_link_libraries(SceneTest PUBLIC Optick)
target_link_libraries(SceneTest PUBLIC SceneComponents)
target_link_libraries(SceneTest PUBLIC Scene)