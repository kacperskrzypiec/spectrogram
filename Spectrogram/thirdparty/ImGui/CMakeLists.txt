cmake_minimum_required(VERSION 3.22)
project(ImGui)

add_library(ImGui)

target_sources(ImGui PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/src/imgui.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_draw.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_tables.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_widgets.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/imgui-SFML.cpp"
)

target_include_directories(ImGui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(ImGui PUBLIC 
	sfml-graphics
	sfml-audio
	sfml-main
	sfml-system
	sfml-window
)