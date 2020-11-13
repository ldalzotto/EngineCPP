cmake_minimum_required(VERSION 3.12)
project(Render VERSION 0.0.1)

file(GLOB_RECURSE RENDER_HEADER "./Render/*.hpp" "./interface/*.hpp" "./driver/*.hpp")
file(GLOB_RECURSE RENDER_SOURCE "./Render/include.cpp" "./interface/*.cpp" "./driver/*.cpp")

enable_language(CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(Render ${RENDER_HEADER} ${RENDER_SOURCE})

target_include_directories(Render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/interface ${CMAKE_CURRENT_SOURCE_DIR}/driver)

target_link_libraries(Render PUBLIC GLFW)
target_link_libraries(Render PUBLIC Vulkan)
target_link_libraries(Render PUBLIC Math)
target_link_libraries(Render PUBLIC Common)
target_link_libraries(Render PUBLIC Optick)
target_link_libraries(Render PUBLIC Assimp)
target_link_libraries(Render PUBLIC AssetServer)