cmake_minimum_required(VERSION 3.12)
project(Test VERSION 0.0.1)

file(GLOB_RECURSE TEST_APP_PRIVATE "./src/*.cpp")
file(GLOB_RECURSE TEST_APP_PUBLIC "./src/*.hpp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(Test "src/EngineCPP.cpp" ${TEST_APP_PUBLIC} ${TEST_APP_PRIVATE})
target_link_libraries(Test PUBLIC Engine)


project(rel001)
add_executable(rel001 "rel/0.0.1.cpp")
target_link_libraries(rel001 PUBLIC Engine)