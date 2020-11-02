cmake_minimum_required(VERSION 3.12)
project(GLFW VERSION 1.0.0)

add_library(GLFW INTERFACE)

target_include_directories(GLFW INTERFACE "public/")
target_link_libraries(GLFW INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw3.lib")

target_link_libraries(GLFW INTERFACE Vulkan)
