cmake_minimum_required(VERSION 3.12)
project(Assimp VERSION 1.0.0)

SET(ASSIMP_DLL_FOLDER ${ASSIMP_DLL_FOLDER} "${CMAKE_CURRENT_SOURCE_DIR}/dll")
SET(ASSIMP_DLL_FILENAME ${ASSIMP_DLL_FILENAME} "assimp-vc142-mt.dll")

add_library(Assimp INTERFACE)
target_include_directories(Assimp INTERFACE "public/")
target_link_libraries(Assimp INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/lib/assimp-vc142-mt.lib")

function(copy_assimp_dll target)
	message(STATUS "Copying ASSIMP dll to ${target}")
	file(COPY ${CMAKE_SOURCE_DIR}/ThirdParty/Assimp/dll/assimp-vc142-mt.dll DESTINATION ${target})
endfunction()