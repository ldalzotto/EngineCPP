cmake_minimum_required(VERSION 3.12)
project(AssetServer VERSION 0.0.1)

file(GLOB_RECURSE ASSETSERVER_HEADER "./AssetServer/*.hpp" "./interface/*.hpp")
file(GLOB_RECURSE ASSETSERVER_SOURCE "./AssetServer/*.cpp" "./AssetServer/include.cpp" "./interface/*.cpp")

enable_language(CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(AssetServer ${ASSETSERVER_HEADER} ${ASSETSERVER_SOURCE})

target_include_directories(AssetServer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/interface)

target_link_libraries(AssetServer PUBLIC Common)
target_link_libraries(AssetServer PUBLIC Optick)
target_link_libraries(AssetServer PUBLIC SQLite)

project(AssetServerTest VERSION 0.0.1)

add_executable(AssetServerTest ${SCENE_HEADER} "./test/test.cpp")

target_include_directories(AssetServerTest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/interface)

target_link_libraries(AssetServerTest PUBLIC Common)
target_link_libraries(AssetServerTest PUBLIC Optick)
target_link_libraries(AssetServerTest PUBLIC SQLite)