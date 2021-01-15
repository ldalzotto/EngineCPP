cmake_minimum_required(VERSION 3.12)
project(EngineCPP VERSION 0.0.1)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(ASSETPATH_DEV=1)

if("${CMAKE_CONFIGURATION_TYPES}" STREQUAL "Debug")
	add_compile_definitions(CONTAINER_BOUND_TEST=1)
	add_compile_definitions(CONTAINER_MEMORY_TEST=1)
	add_compile_definitions(MATH_NORMALIZATION_TEST=1)
	add_compile_definitions(SCENE_BOUND_TEST=1)
	add_compile_definitions(RENDER_BOUND_TEST=1)
	add_compile_definitions(COLLIDER_BOUND_TEST=1)
	add_compile_definitions(STANDARD_ALLOCATION_BOUND_TEST=1)
	add_compile_definitions(USE_OPTICK=0)
else()
	add_compile_definitions(CONTAINER_BOUND_TEST=0)
	add_compile_definitions(CONTAINER_MEMORY_TEST=0)
	add_compile_definitions(MATH_BOUND_TEST=0)
	add_compile_definitions(SCENE_BOUND_TEST=0)
	add_compile_definitions(RENDER_BOUND_TEST=0)
	add_compile_definitions(COLLIDER_BOUND_TEST=0)
	add_compile_definitions(STANDARD_ALLOCATION_BOUND_TEST=0)
	add_compile_definitions(USE_OPTICK=0)
endif()


add_subdirectory(ThirdParty)

add_subdirectory(Common)
add_subdirectory(Math)
add_subdirectory(AssetServer)
add_subdirectory(Render)
add_subdirectory(Collision)
add_subdirectory(Input)
add_subdirectory(Scene)
add_subdirectory(Middleware)
add_subdirectory(Engine)

add_subdirectory(SceneSerialization)
add_subdirectory(Tools)
add_subdirectory(CPPTestVS)


add_subdirectory(Rewrite/Common2)
add_subdirectory(Rewrite/Math2)

add_subdirectory(Test)