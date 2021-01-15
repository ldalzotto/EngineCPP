cmake_minimum_required(VERSION 3.12)
project(Collision VERSION 0.0.1)

file(GLOB_RECURSE COLLISION_HEADER "./interface/*.hpp")
file(GLOB_RECURSE COLLISION_SOURCE "./Collision/collision.cpp")

enable_language(CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(Collision ${COLLISION_HEADER} ${COLLISION_SOURCE})

target_include_directories(Collision PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/interface ${CMAKE_CURRENT_SOURCE_DIR}/driver)

target_link_libraries(Collision PUBLIC Math2)
target_link_libraries(Collision PUBLIC Common2)



add_executable(CollisionTest "./test/test.cpp")
target_include_directories(CollisionTest INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CollisionTest PUBLIC Collision)