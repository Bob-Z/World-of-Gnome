cmake_minimum_required(VERSION 2.8)
enable_language(CXX)
enable_language(C)

project(common)
set(VERSION 0.0.0)

find_package(Lua50 REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

add_library(
	${PROJECT_NAME}
	SHARED
	context.cpp
	entry.cpp
	fifo.cpp
	file.cpp
	font.cpp
	item.cpp
	list.cpp
	log.cpp
	map.cpp
	mutex.cpp
	network.cpp
	sfx.cpp
	util.cpp
	lua.cpp
	NetworkFrame.cpp
)

target_link_libraries (${PROJECT_NAME}
        ${LUA_LIBRARIES}
)
