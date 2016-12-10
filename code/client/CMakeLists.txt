cmake_minimum_required(VERSION 2.8)
enable_language(CXX)
enable_language(C)

project(client)
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

	client.cpp
	file_client.cpp
	imageDB.cpp
	main_client.cpp
	network_client.cpp
	option_client.cpp
	parser_client.cpp
	screen.cpp
	scr_play.cpp
	scr_select.cpp
	textview.cpp
	ui_play.cpp
)

