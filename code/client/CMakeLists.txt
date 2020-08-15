cmake_minimum_required(VERSION 2.8)
enable_language(CXX)
enable_language(C)

project(wog_client)
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
include_directories(${COMMON_PATH})
include_directories(${COMMON_PATH}/Context)
include_directories(${COMMON_PATH}/Network)

set(SDL_ITEM_PATH ../sdl_item/)
include_directories(${SDL_ITEM_PATH}/include)

include_directories(.)
include_directories(ui)

add_executable(
	${PROJECT_NAME}	

	Camera.cpp
	CharacterMarquee.cpp
	client.cpp
	client_conf.cpp
	DataManager/DataManagerClient.cpp
	DataManager/FileStatus.cpp
	EffectManager.cpp
	file_client.cpp
	imageDB.cpp
	lua_client.cpp
	main_client.cpp
	network_client.cpp
	parser_client.cpp
	screen.cpp
	scr_play.cpp
	scr_select.cpp
	scr_create.cpp
	sfx.cpp
	textview.cpp
	ui_play.cpp
	ui/ChatBox.cpp
	ui/ChatEditBox.cpp
	ui/ChatHistoryBox.cpp
	${PROTO_SRCS} ${PROTO_HDRS}
)

target_link_libraries (
	${PROJECT_NAME}
	
	${LUA_LIBRARIES}
	common
	${SDL2_LIBRARIES}
	${SDL2NET_LIBRARIES}
	${SDL2MIXER_LIBRARIES}
	${SDL2TTF_LIBRARIES}
	${CONFIG_LIBRARIES}
	sdl_item
	${Protobuf_LIBRARIES}
)
