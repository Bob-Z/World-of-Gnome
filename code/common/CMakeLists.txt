cmake_minimum_required(VERSION 2.8)
enable_language(CXX)
enable_language(C)

add_subdirectory(proto)

project(common)
set(VERSION 0.0.0)

find_package(PkgConfig REQUIRED)

pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2NET REQUIRED SDL2_net>=2.0.0)
include_directories(
	${SDL2_INCLUDE_DIRS}
        ${SDL2NET_INCLUDE_DIRS}
)

# for LUA
find_package(Lua50 REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

#for protobuf
include(FindProtobuf)
find_package(Protobuf REQUIRED)

# for generated protobuf
include_directories(${CMAKE_BINARY_DIR}/common/proto)

include_directories(.)
include_directories(./Context)
include_directories(./Network)

add_library(
	${PROJECT_NAME}
	SHARED

	${PROTO_HEADER}
	${PROTO_SRC}

	Context/Context.cpp
	Context/ContextContainer.cpp
	Context/ContextGetter.cpp
	entry.cpp
	file.cpp
	font.cpp
	item.cpp
	list.cpp
	Lock.cpp
	LockGuard.cpp
	log.cpp
	lua_script.cpp
	map.cpp
	mutex.cpp
	Network/Connection.cpp
	Network/DataSent.cpp
	Network/network.cpp
	RunningAction.cpp
	Serializer.cpp
	Selection.cpp
	util.cpp
)

target_link_libraries (${PROJECT_NAME}
        ${LUA_LIBRARIES}
        proto
        ${PROTOBUF_LIBRARY}
	${SDL2_LIBRARIES}
	${SDL2NET_LIBRARIES}
)
