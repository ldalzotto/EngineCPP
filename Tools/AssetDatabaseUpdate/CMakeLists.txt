if(USE_TOOL)

cmake_minimum_required(VERSION 3.12)
project(AssetDatabaseUpdate VERSION 0.0.1)

file(GLOB_RECURSE ASSET_DB_UPDATE_HEADER "./AssetDatabaseUpdate/*.hpp")
file(GLOB_RECURSE ASSET_DB_UPDATE_SOURCE "./AssetDatabaseUpdate/*.cpp")

enable_language(CXX)

# LIST(APPEND CMAKE_PROGRAM_PATH  "$ENV{QT_SDK}/bin/" "$ENV{QT_SDK}/plugins/")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_executable(AssetDatabaseUpdate ${ASSET_DB_UPDATE_HEADER} ${ASSET_DB_UPDATE_SOURCE})



if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(CMAKE_PREFIX_PATH "$ENV{QT_SDK}/lib/cmake")
find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui)

target_link_libraries(AssetDatabaseUpdate Common)
target_link_libraries(AssetDatabaseUpdate Math)
target_link_libraries(AssetDatabaseUpdate Render)
target_link_libraries(AssetDatabaseUpdate AssetServer)
target_link_libraries(AssetDatabaseUpdate Scene)
target_link_libraries(AssetDatabaseUpdate SceneComponents)
target_link_libraries(AssetDatabaseUpdate Qt5::Widgets Qt5::Core Qt5::Gui)
target_link_libraries(AssetDatabaseUpdate StbImage)

copy_qt_dll(${PROJECT_BINARY_DIR}/${BUILD_TYPE}${CMAKE_CONFIGURATION_TYPES})

endif()
