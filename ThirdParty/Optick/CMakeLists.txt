cmake_minimum_required(VERSION 3.12)
project(Optick VERSION 0.0.1)

file(GLOB_RECURSE Optick_HEADER "./public/*.h")

add_library(Optick INTERFACE)

target_include_directories(Optick INTERFACE "public/")
target_link_libraries(Optick INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/lib/Optick.lib")

target_link_libraries(Optick INTERFACE Vulkan)