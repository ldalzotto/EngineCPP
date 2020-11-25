cmake_minimum_required(VERSION 3.12)
project(MainTool VERSION 0.0.1)

file(GLOB_RECURSE MAIN_TOOL_HEADER "./MainTool/*.hpp")
file(GLOB_RECURSE MAIN_TOOL_SOURCE "./MainTool/*.cpp")

enable_language(CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_executable(MainTool ${MAIN_TOOL_HEADER} ${MAIN_TOOL_SOURCE})

target_link_libraries(MainTool Common)
target_link_libraries(MainTool Math)
target_link_libraries(MainTool Render)
target_link_libraries(MainTool AssetServer)
target_link_libraries(MainTool Scene)
target_link_libraries(MainTool SceneComponents)
target_link_libraries(MainTool Middleware)
target_link_libraries(MainTool StbImage)
target_link_libraries(MainTool Engine)