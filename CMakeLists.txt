cmake_minimum_required(VERSION 3.12)
project(EngineCPP VERSION 0.0.1)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(ThirdParty)
add_subdirectory(Common)
add_subdirectory(Math)
add_subdirectory(Render)
add_subdirectory(Engine)
add_subdirectory(CPPTestVS)
