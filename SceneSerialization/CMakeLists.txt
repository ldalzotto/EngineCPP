cmake_minimum_required(VERSION 3.12)
project(SceneSerialization VERSION 0.0.1)

file(GLOB_RECURSE SCENE_SERIALIZATION_HEADER "./SceneSerialization/*.hpp" "./interface/*.hpp")
file(GLOB_RECURSE SCENE_SERIALIZATION_SOURCE "./SceneSerialization/*.cpp" "./SceneSerialization/include.cpp" "./interface/*.cpp")

enable_language(CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(SceneSerialization ${SCENE_SERIALIZATION_HEADER} ${SCENE_SERIALIZATION_SOURCE})

target_include_directories(SceneSerialization PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/interface)

target_link_libraries(SceneSerialization PUBLIC Math)
target_link_libraries(SceneSerialization PUBLIC Common)
target_link_libraries(SceneSerialization PUBLIC Scene)
target_link_libraries(SceneSerialization PUBLIC SceneComponents)
target_link_libraries(SceneSerialization PUBLIC AssetServer)
target_link_libraries(SceneSerialization PUBLIC Optick)
