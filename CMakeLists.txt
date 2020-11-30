cmake_minimum_required(VERSION 3.12)
project(EngineCPP VERSION 0.0.1)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(USE_OPTICK=0)
add_compile_definitions(ASSETPATH_DEV=1)

add_subdirectory(ThirdParty)
add_subdirectory(Common)
add_subdirectory(Math)
add_subdirectory(AssetServer)
add_subdirectory(Render)
add_subdirectory(Input)
add_subdirectory(Scene)
add_subdirectory(Middleware)
add_subdirectory(Engine)

add_subdirectory(SceneSerialization)
add_subdirectory(Tools)
add_subdirectory(CPPTestVS)
