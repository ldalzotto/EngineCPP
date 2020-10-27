cmake_minimum_required(VERSION 3.12)
project(Vulkan VERSION 1.0.0)

add_library(Vulkan INTERFACE)

file(GLOB_RECURSE VULKAN_LIB "$ENV{VULKAN_SDK}/lib/*.lib")

target_include_directories(Vulkan INTERFACE "$ENV{VULKAN_SDK}/Include/" "public/")
target_link_libraries(Vulkan INTERFACE "${VULKAN_LIB}")