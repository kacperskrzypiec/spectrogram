cmake_minimum_required(VERSION 3.22)
project(Spectrogram VERSION 1.0)
set(TARGET_NAME "Spectrogram")

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

set(SFML_USE_STATIC_STD_LIBS ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

add_subdirectory(thirdparty/SFML-2.6.1)
add_subdirectory(thirdparty/ImGui)

add_executable("${TARGET_NAME}" icon.rc)
target_compile_definitions("${TARGET_NAME}" PUBLIC IDI_ICON1=102) 

set(DllDir "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SFML-2.6.1/extlibs/bin/x64/")
add_custom_command(TARGET "${TARGET_NAME}" POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${DllDir}/openal32.dll"
	"$<TARGET_FILE_DIR:${TARGET_NAME}>"
)

if(MSVC)
	target_compile_definitions("${TARGET_NAME}" PUBLIC _CRT_SECURE_NO_WARNINGS)

	if(PRODUCTION_BUILD)
		set_target_properties("${TARGET_NAME}" PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
	endif()
endif()

target_compile_definitions("${TARGET_NAME}" PUBLIC CPP_DLL_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../CppLibrary/bin/${CMAKE_BUILD_TYPE}/") 
target_compile_definitions("${TARGET_NAME}" PUBLIC ASM_DLL_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../AsmLibrary/bin/${CMAKE_BUILD_TYPE}/") 

if(PRODUCTION_BUILD)
	target_compile_definitions("${TARGET_NAME}" PUBLIC DATA_PATH="./data/") 
	target_compile_definitions("${TARGET_NAME}" PUBLIC PRODUCTION_BUILD=1) 
else()
	target_compile_definitions("${TARGET_NAME}" PUBLIC DATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/")
	target_compile_definitions("${TARGET_NAME}" PUBLIC PRODUCTION_BUILD=0) 
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
	target_compile_definitions("${TARGET_NAME}" PUBLIC NDEBUG) 
endif()

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

target_sources("${TARGET_NAME}" PRIVATE ${MY_SOURCES})

target_include_directories("${TARGET_NAME}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_include_directories("${TARGET_NAME}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SFML-2.6.1/include/")
target_include_directories("${TARGET_NAME}" SYSTEM PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ImGui/include/")

target_link_libraries("${TARGET_NAME}" PRIVATE
	ImGui
	sfml-graphics
	sfml-audio
	sfml-main
	sfml-system
	sfml-window
)