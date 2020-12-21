cmake_minimum_required(VERSION 3.12)
project(Engine VERSION 0.0.1)

file(GLOB_RECURSE RENDER_HEADER "./Engine/*.hpp" "./interface/*.hpp" "./driver/*.hpp")
file(GLOB_RECURSE RENDER_SOURCE "./Engine/*.cpp" "./interface/*.cpp" "./driver/*.cpp")

enable_language(CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(Engine ${RENDER_HEADER} ${RENDER_SOURCE})

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/interface)

target_link_libraries(Engine PUBLIC GLFW)
target_link_libraries(Engine PUBLIC Optick)
target_link_libraries(Engine PUBLIC AssetServer)
target_link_libraries(Engine PUBLIC Render)
target_link_libraries(Engine PUBLIC Collision)
target_link_libraries(Engine PUBLIC Input)
target_link_libraries(Engine PUBLIC Scene)
target_link_libraries(Engine PUBLIC SceneComponents)
target_link_libraries(Engine PUBLIC SceneSerialization)
target_link_libraries(Engine PUBLIC Middleware)
