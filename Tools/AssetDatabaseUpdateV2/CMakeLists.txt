cmake_minimum_required(VERSION 3.12)
project(AssetDatabaseUpdateV2 VERSION 0.0.1)

file(GLOB_RECURSE ASSET_DB_UPDATE_HEADER "./AssetDatabaseUpdateV2/*.hpp")
file(GLOB_RECURSE ASSET_DB_UPDATE_SOURCE "./AssetDatabaseUpdateV2/*.cpp")

enable_language(CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_executable(AssetDatabaseUpdateV2 ${ASSET_DB_UPDATE_HEADER} ${ASSET_DB_UPDATE_SOURCE})

target_link_libraries(AssetDatabaseUpdateV2 Common)
target_link_libraries(AssetDatabaseUpdateV2 Math)
target_link_libraries(AssetDatabaseUpdateV2 Render)
target_link_libraries(AssetDatabaseUpdateV2 AssetServer)
target_link_libraries(AssetDatabaseUpdateV2 Scene)
target_link_libraries(AssetDatabaseUpdateV2 SceneComponents)
target_link_libraries(AssetDatabaseUpdateV2 Middleware)
target_link_libraries(AssetDatabaseUpdateV2 StbImage)