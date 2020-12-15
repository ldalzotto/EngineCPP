cmake_minimum_required(VERSION 3.12)
project(SceneSerialization VERSION 0.0.1)

enable_language(CXX)

add_library(SceneSerialization INTERFACE)
target_include_directories(SceneSerialization INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/interface)

target_link_libraries(SceneSerialization INTERFACE Math)
target_link_libraries(SceneSerialization INTERFACE Common)
target_link_libraries(SceneSerialization INTERFACE Scene)
target_link_libraries(SceneSerialization INTERFACE SceneComponents)
target_link_libraries(SceneSerialization INTERFACE AssetServer)
target_link_libraries(SceneSerialization INTERFACE Optick)
