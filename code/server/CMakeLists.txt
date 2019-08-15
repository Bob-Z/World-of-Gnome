cmake_minimum_required(VERSION 2.8)
enable_language(CXX)
enable_language(C)

project(wog_server)
set(VERSION 0.0.0)

find_package(PkgConfig REQUIRED)

pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2NET REQUIRED SDL2_net>=2.0.0)
pkg_search_module(SDL2MIXER REQUIRED SDL2_mixer>=2.0.0)
pkg_search_module(SDL2TTF REQUIRED SDL2_ttf>=2.0.12)
include_directories(
	${SDL2_INCLUDE_DIRS}
	${SDL2NET_INCLUDE_DIRS}
	${SDL2MIXER_INCLUDE_DIRS}
	${SDL2TTF}
)

pkg_search_module(CONFIG REQUIRED libconfig)
include_directories(${CONFIG_INCLUDE_DIRS})

find_package(Lua50 REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

find_package(Protobuf REQUIRED)
include_directories( ${Protobuf_INCLUDE_DIRS} )
include_directories(${CMAKE_BINARY_DIR}/common/proto)

set(COMMON_PATH ../common/)
include_directories( ${COMMON_PATH} )

add_executable(
	${PROJECT_NAME}

	action.cpp
	attribute.cpp
	character.cpp
	context_server.cpp
	equipment.cpp
	inventory.cpp
	main_server.cpp
	map_server.cpp
	network_server.cpp
	npc.cpp
	parser_server.cpp
	server.cpp
	${PROTO_SRCS} ${PROTO_HDRS}
)

target_link_libraries (${PROJECT_NAME}
	${LUA_LIBRARIES}
	common
	${SDL2_LIBRARIES}
	${SDL2NET_LIBRARIES}
	${SDL2MIXER_LIBRARIES}
	${SDL2TTF_LIBRARIES}
	${CONFIG_LIBRARIES}
	${Protobuf_LIBRARIES}
)

