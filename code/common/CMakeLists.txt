cmake_minimum_required(VERSION 2.8)
enable_language(CXX)
enable_language(C)

add_subdirectory(proto)

project(common)
set(VERSION 0.0.0)

# for LUA
find_package(Lua50 REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

#for protobuf
include(FindProtobuf)
find_package(Protobuf REQUIRED)

# for generated protobuf
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(
	${PROJECT_NAME}
	SHARED

	${PROTO_HEADER}
    ${PROTO_SRC}

	context.cpp
	entry.cpp
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
	Deserializer.cpp
)

target_link_libraries (${PROJECT_NAME}
        ${LUA_LIBRARIES}
        proto
        ${PROTOBUF_LIBRARY}
)
