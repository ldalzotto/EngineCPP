project(SandboxTest VERSION 0.0.1)

add_executable(SandboxTest "./SandboxTest.cpp")

target_include_directories(SandboxTest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/interface)

target_link_libraries(SandboxTest PUBLIC Math)
target_link_libraries(SandboxTest PUBLIC Common)
target_link_libraries(SandboxTest PUBLIC Optick)
target_link_libraries(SandboxTest PUBLIC SceneComponents)