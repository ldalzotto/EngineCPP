if(USE_TOOL)

cmake_minimum_required(VERSION 3.12)
project(AssetDatabaseUpdate VERSION 0.0.1)

file(GLOB_RECURSE ASSET_DB_UPDATE_HEADER "./AssetDatabaseUpdate/*.hpp")
file(GLOB_RECURSE ASSET_DB_UPDATE_SOURCE "./AssetDatabaseUpdate/*.cpp")

enable_language(CXX)

# LIST(APPEND CMAKE_PROGRAM_PATH  "$ENV{QT_SDK}/bin/" "$ENV{QT_SDK}/plugins/")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_executable(AssetDatabaseUpdate ${ASSET_DB_UPDATE_HEADER} ${ASSET_DB_UPDATE_SOURCE})

target_link_libraries(AssetDatabaseUpdate Qt)


copy_qt_dll(${PROJECT_BINARY_DIR}/${BUILD_TYPE}${CMAKE_CONFIGURATION_TYPES})

endif()
