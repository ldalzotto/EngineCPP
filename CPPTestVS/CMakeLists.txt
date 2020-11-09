cmake_minimum_required(VERSION 3.12)
project(Test VERSION 0.0.1)

file(GLOB_RECURSE TEST_APP_PRIVATE "./src/*.cpp")
file(GLOB_RECURSE TEST_APP_PUBLIC "./src/*.hpp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(Test "src/EngineCPP.cpp" ${TEST_APP_PUBLIC} ${TEST_APP_PRIVATE})

target_link_libraries(Test PUBLIC Common)
target_link_libraries(Test PUBLIC Math)
target_link_libraries(Test PUBLIC SceneComponents)
target_link_libraries(Test PUBLIC Engine)


copy_assimp_dll(${PROJECT_BINARY_DIR}/${BUILD_TYPE}${CMAKE_CONFIGURATION_TYPES})