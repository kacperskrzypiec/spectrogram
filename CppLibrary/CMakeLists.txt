cmake_minimum_required(VERSION 3.22)
project(CppLibrary VERSION 1.0)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(TARGET_NAME "CppLibrary-d")
else()
    set(TARGET_NAME "CppLibrary")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

if(PRODUCTION_BUILD)
	set(FullOutputDir "${CMAKE_CURRENT_SOURCE_DIR}/bin/Production")
else()
	set(FullOutputDir "${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
endif()
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${FullOutputDir}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${FullOutputDir}")

if(MSVC) 
	add_compile_options(/arch:AVX2)
	add_compile_options(/utf-8)

	if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
        add_definitions(/MP)
    endif()
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
endif()

add_library(${TARGET_NAME} SHARED
    src/main.cpp
    src/Functions.cpp
)