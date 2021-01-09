cmake_minimum_required(VERSION 3.12)
project(Collision2 VERSION 0.0.1)

file(GLOB_RECURSE COLLISION2_HEADER "./interface/*.hpp")
file(GLOB_RECURSE COLLISION2_SOURCE "./Collision2/collision2.cpp")

enable_language(CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(Collision2 ${COLLISION2_HEADER} ${COLLISION2_SOURCE})

target_include_directories(Collision2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/interface ${CMAKE_CURRENT_SOURCE_DIR}/driver)

target_link_libraries(Collision2 PUBLIC Math)
target_link_libraries(Collision2 PUBLIC Common2)
target_link_libraries(Collision2 PUBLIC AssetServer)



add_executable(Collision2Test "./test/test.cpp")
target_include_directories(Collision2Test INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Collision2Test PUBLIC Collision2)