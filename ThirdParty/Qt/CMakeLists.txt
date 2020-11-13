if(USE_TOOL)

project(Qt VERSION 1.0.0)

add_library(Qt INTERFACE)

file(GLOB_RECURSE QT_LIB "$ENV{QT_SDK}/lib/*.lib")

target_include_directories(Qt INTERFACE "$ENV{QT_SDK}/include/")
target_link_libraries(Qt INTERFACE "${QT_LIB}")

function(copy_qt_dll target)
	file(GLOB Qt_DLL "$ENV{QT_SDK}/bin/*.dll" "$ENV{QT_SDK}/plugins/platforms")
	message(STATUS "Copying QT dll to ${target}")
	file(COPY ${Qt_DLL} DESTINATION ${target})
endfunction()

endif()