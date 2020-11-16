if(USE_TOOL)

cmake_minimum_required(VERSION 3.12)
project(StbImage VERSION 0.0.1)

add_library(StbImage INTERFACE)

target_include_directories(StbImage INTERFACE "public/")

endif()