cmake_minimum_required(VERSION 2.8)
enable_language(CXX)
enable_language(C)

project(World of Gnome)
set(VERSION 0.0.0)

#find_package(PkgConfig REQUIRED)
#pkg_search_module(....)

#Path
#set(COMMON_PATH ${CMAKE_CURRENT_SOURCE_DIR}/common/)
#set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_YPE})
#set(LIBRARY_OUTPUT_PATH ${COMMON_PATH}/lib/${CMAKE_BUILD_YPE})

#Compile
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11"


include_directories( ${COMMON_PATH} )

add_executable(
	wog_client

	client.c
	file_client.c
	imageDB.c
	main_client.c
	network_client.c
	option_client.c
	parser_client.c
	screen.c
	scr_play.c
	scr_select.c
	textview.c
	ui_play.c
)

