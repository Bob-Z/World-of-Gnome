/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2017-2020 carabobz@gmail.com

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include "Context.h"

#include "Camera.h"
#include "entry.h"
#include "imageDB.h"
#include "item.h"
#include "log.h"
#include "sdl.h"
#include "SdlItem.h"
#include "SdlLocking.h"
#include "sfx.h"
#include "syntax.h"
#include "util.h"

#ifdef __cplusplus
extern "C"
{
#endif
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#ifdef __cplusplus
}
#endif

static lua_State * luaVm = nullptr;
static SDL_mutex* luaVmMutex = SDL_CreateMutex();
static lua_State * effectLuaVm = nullptr;
static SDL_mutex* effectLuaVmMutex = SDL_CreateMutex();

/***********************************
 player_get_id
 Input:
 Output: ID of the current context
 ***********************************/
static int l_player_get_id(lua_State* luaState)
{
	Context * context;

	context = context_get_player();
	lua_pushstring(luaState, context->getId().c_str());
	return 1;  // number of results
}

/***********************************
 context_get_id
 Input:
 Output: ID of current context
 ***********************************/
static int l_context_get_id(lua_State* luaState)
{
	Context * context;

	lua_getglobal(luaState, "current_context");
	context = (Context*) lua_touserdata(luaState, -1);
	lua_pop(luaState, 1);

	lua_pushstring(luaState, context->getId().c_str());
	return 1; // number of results
}

/***********************************
 context_get_npc
 Input:
 Output: 1 if context is an NPC
 ***********************************/
static int l_context_get_npc(lua_State* luaState)
{
	Context * context;

	lua_getglobal(luaState, "current_context");
	context = (Context*) lua_touserdata(luaState, -1);
	lua_pop(luaState, 1);

	lua_pushnumber(luaState, context->isNpc());
	return 1; // number of results
}

/***********************************
 context_get_map
 Input:
 Output: current map name
 ***********************************/
static int l_context_get_map(lua_State* luaState)
{
	Context * context;

	lua_getglobal(luaState, "current_context");
	context = (Context*) lua_touserdata(luaState, -1);
	lua_pop(luaState, 1);

	lua_pushstring(luaState, context->getMap().c_str());
	return 1; // number of results
}

/***********************************
 item_set_px
 Input: X ccordinate in pixel
 Output:
 ***********************************/
static int l_item_set_px(lua_State* luaState)
{
	SdlItem * item;

	lua_getglobal(luaState, "current_item");
	item = (SdlItem*) lua_touserdata(luaState, -1);
	lua_pop(luaState, 1);

	int x;
	x = luaL_checkint(luaState, -1);
	item->setRectX(x);
	return 0; // number of results
}

/***********************************
 item_set_py
 Input: Y ccordinate in pixel
 Output:
 ***********************************/
static int l_item_set_py(lua_State* luaState)
{
	SdlItem * item;

	lua_getglobal(luaState, "current_item");
	item = (SdlItem*) lua_touserdata(luaState, -1);
	lua_pop(luaState, 1);

	int y;
	y = luaL_checkint(luaState, -1);
	item->setRectY(y);
	return 0; // number of results
}

/***********************************
 item_get_px
 Input:
 Output: X coordinate in pixel
 ***********************************/
static int l_item_get_px(lua_State* luaState)
{
	SdlItem * item;

	lua_getglobal(luaState, "current_item");
	item = (SdlItem*) lua_touserdata(luaState, -1);
	lua_pop(luaState, 1);

	lua_pushnumber(luaState, item->getRect().x);
	return 1; // number of results
}

/***********************************
 item_get_py
 Input:
 Output: Y ccordinate in pixel
 ***********************************/
static int l_item_get_py(lua_State* luaState)
{
	SdlItem * item;

	lua_getglobal(luaState, "current_item");
	item = (SdlItem*) lua_touserdata(luaState, -1);
	lua_pop(luaState, 1);

	lua_pushnumber(luaState, item->getRect().y);
	return 1; // number of results
}

/***********************************
 item_get_w
 Input:
 Output: Width in pixel
 ***********************************/
static int l_item_get_w(lua_State* luaState)
{
	SdlItem * item;

	lua_getglobal(luaState, "current_item");
	item = (SdlItem*) lua_touserdata(luaState, -1);
	lua_pop(luaState, 1);

	lua_pushnumber(luaState, item->getRect().w);
	return 1; // number of results
}

/***********************************
 item_get_h
 Input:
 Output: Height in pixel
 ***********************************/
static int l_item_get_h(lua_State* luaState)
{
	SdlItem * item;

	lua_getglobal(luaState, "current_item");
	item = (SdlItem*) lua_touserdata(luaState, -1);
	lua_pop(luaState, 1);

	lua_pushnumber(luaState, item->getRect().h);
	return 1; // number of results
}
/***********************************
 item_set_anim_start_tick
 Input: Tick from when animation will be calculated
 Output:
 ***********************************/
static int l_item_set_anim_start_tick(lua_State* luaState)
{
	SdlItem * item;

	lua_getglobal(luaState, "current_item");
	item = (SdlItem*) lua_touserdata(luaState, -1);
	lua_pop(luaState, 1);

	int tick;
	tick = luaL_checkint(luaState, -1);
	item->setAnimStartTick(tick);

	return 0; // number of results
}

/***********************************
 item_set_anim_from a file name
 Input:
 - file name
 Output:
 ***********************************/
static int l_item_set_anim(lua_State* luaState)
{
	SdlItem * item;
	lua_getglobal(luaState, "current_item");
	item = (SdlItem*) lua_touserdata(luaState, -1);
	lua_pop(luaState, 1);

	const char * fileName = luaL_checkstring(luaState, -1);

	Anim * anim = imageDB_get_anim(context_get_player(), std::string(fileName));
	item->setAnim(anim);

	return 0; // number of results
}

/*****************************************************************************/
static const std::string & getKey(const bool isMoving, const char orientation)
{
	if (isMoving == true)
	{
		switch (orientation)
		{
		case 'N':
			return CHARACTER_KEY_MOV_N_SPRITE;
			break;
		case 'S':
			return CHARACTER_KEY_MOV_S_SPRITE;
			break;
		case 'W':
			return CHARACTER_KEY_MOV_W_SPRITE;
			break;
		case 'E':
			return CHARACTER_KEY_MOV_E_SPRITE;
			break;
		default:
			ERR("l_item_set_anim_from_context: wrong main orientation");
			return CHARACTER_KEY_MOV_S_SPRITE;
			break;
		}
	}
	else
	{
		switch (orientation)
		{
		case 'N':
			return CHARACTER_KEY_DIR_N_SPRITE;
			break;
		case 'S':
			return CHARACTER_KEY_DIR_S_SPRITE;
			break;
		case 'W':
			return CHARACTER_KEY_DIR_W_SPRITE;
			break;
		case 'E':
			return CHARACTER_KEY_DIR_E_SPRITE;
			break;
		default:
			ERR("l_item_set_anim_from_context: wrong main orientation");
			return CHARACTER_KEY_DIR_S_SPRITE;
			break;
		}
	}
}

/*****************************************************************************/
static std::vector<Anim*> getAnimArray(const std::string & id, const std::string & key)
{
	std::vector<Anim*> animArray;

	// Try single image anim
	char * spriteName = nullptr;
	if (entry_read_string(CHARACTER_TABLE, id.c_str(), &spriteName, key.c_str(), nullptr) == true)
	{
		if ((spriteName != nullptr) && (spriteName[0] != 0))
		{
			Anim * anim = imageDB_get_anim(context_get_player(), std::string(spriteName));
			animArray.push_back(anim);

			free(spriteName);
			return animArray;
		}
		free(spriteName);
	}

	// Try list of image
	char ** spriteList = nullptr;
	if (entry_read_list(CHARACTER_TABLE, id.c_str(), &spriteList, key.c_str(), nullptr) == true)
	{
		std::vector<std::string> spriteNameArray;

		int i = 0;
		char* spriteName = nullptr;
		while ((spriteName = spriteList[i]) != nullptr)
		{
			spriteNameArray.push_back(std::string(spriteName));
			i++;
		}

		animArray = imageDB_get_anim_array(context_get_player(), spriteNameArray);
		deep_free(spriteList);
		return animArray;
	}

	return animArray;
}

/*****************************************************************************/
static char flip(char orientation)
{
	char l_FlipOrientation = 'S';

	switch (orientation)
	{
	case 'N':
		l_FlipOrientation = 'S';
		break;
	case 'S':
		l_FlipOrientation = 'N';
		break;
	case 'W':
		l_FlipOrientation = 'E';
		break;
	case 'E':
		l_FlipOrientation = 'W';
		break;
	default:
		break;
	}

	return l_FlipOrientation;
}

/******************************************************************************
 item_set_anim_from_context
 Input:
 - ID of context
 - entry name in context file
 Output:
 *****************************************************************************/
static int l_item_set_anim_from_context(lua_State* luaState)
{
	SdlItem * item;
	lua_getglobal(luaState, "current_item");
	item = (SdlItem*) lua_touserdata(luaState, -1);
	lua_pop(luaState, 1);

	const char * id = luaL_checkstring(luaState, -4);
	int isMoving = luaL_checkint(luaState, -3);
	const char * mainOrientation = luaL_checkstring(luaState, -2);
	const char * secondaryOrientation = luaL_checkstring(luaState, -1);

	// reset previous anim
	item->clearAnim();

	// Try regular moving flag with main orientation
	std::string key = getKey(isMoving, mainOrientation[0]);
	std::vector<Anim*> animArray = getAnimArray(id, key);

	// Try secondary orientation
	if (animArray.size() == 0)
	{
		key = getKey(isMoving, secondaryOrientation[0]);
		animArray = getAnimArray(id, key);
	}

	// Auto-flip
	if (animArray.size() == 0)
	{
		char flipOrientation = flip(mainOrientation[0]);
		key = getKey(isMoving, flipOrientation);
		animArray = getAnimArray(id, key);
	}

	if (animArray.size() == 0)
	{
		char flipOrientation = flip(secondaryOrientation[0]);
		key = getKey(isMoving, flipOrientation);
		animArray = getAnimArray(id, key);
	}

	// Try opposite moving flag
	// main orientation
	if (animArray.size() == 0)
	{
		isMoving = !isMoving;
		key = getKey(isMoving, mainOrientation[0]);
		animArray = getAnimArray(id, key);
	}
	// secondary orientation
	if (animArray.size() == 0)
	{
		key = getKey(isMoving, secondaryOrientation[0]);
		animArray = getAnimArray(id, key);
	}

	// Try default sprite
	if (animArray.size() == 0)
	{
		animArray = getAnimArray(id, CHARACTER_KEY_SPRITE);
	}

	if (animArray.size() == 0)
	{
		ERR_DESIGN("LUA item_set_anim_from_context: Failed to find anim for " + std::string(id));
	}
	else
	{
		item->setAnim(animArray);
	}

	return 0; // number of results
}

/***********************************
 camera_get_screen

 Get current screen shown by camera

 Input:
 Output:
 - screen number
 ***********************************/
static int l_camera_get_screen(lua_State* luaState)
{
	Camera * camera;
	lua_getglobal(luaState, "current_camera");
	camera = (Camera*) lua_touserdata(luaState, -1);
	lua_pop(luaState, 1);

	lua_pushnumber(luaState, (int) camera->getScreen());
	return 1; // number of results
}

/***********************************
 camera_get_zoom

 Get current camera zoom

 Input:
 Output:
 - zoom factor ( +1 for one level of zoom, -1 for UNzoom)
 ***********************************/
static int l_camera_get_zoom(lua_State* luaState)
{
	Camera * camera;
	lua_getglobal(luaState, "current_camera");
	camera = (Camera*) lua_touserdata(luaState, -1);
	lua_pop(luaState, 1);

	lua_pushnumber(luaState, camera->getZoom());
	return 1; // number of results
}

/***********************************
 camera_get_X

 Get current camera X position

 Input:
 Output:
 - X position in pixel
 ***********************************/
static int l_camera_get_X(lua_State* luaState)
{
	Camera * camera;
	lua_getglobal(luaState, "current_camera");
	camera = (Camera*) lua_touserdata(luaState, -1);
	lua_pop(luaState, 1);

	lua_pushnumber(luaState, camera->getX());
	return 1; // number of results
}

/***********************************
 camera_get_Y

 Get current camera Y position

 Input:
 Output:
 - Y position in pixel
 ***********************************/
static int l_camera_get_Y(lua_State* luaState)
{
	Camera * camera;
	lua_getglobal(luaState, "current_camera");
	camera = (Camera*) lua_touserdata(luaState, -1);
	lua_pop(luaState, 1);

	lua_pushnumber(luaState, camera->getY());
	return 1; // number of results
}

/***********************************
 camera_set_coord

 Set camera coordinate

 Input:
 - X
 - Y
 Output:
 ***********************************/
static int l_camera_set_coord(lua_State* luaState)
{
	int x;
	x = luaL_checknumber(luaState, -2);
	int y;
	y = luaL_checknumber(luaState, -1);

	sdl_force_virtual_x(x);
	sdl_force_virtual_y(y);

	return 0; // number of results
}

/***********************************
 camera_set_zoom

 Set camera zoom

 Input:
 - Zoom level (1.0 = 100%)
 Output:
 ***********************************/
static int l_camera_set_zoom(lua_State* luaState)
{
	double l_Zoom;
	l_Zoom = luaL_checknumber(luaState, -1);

	sdl_force_virtual_z(l_Zoom);

	return 0; // number of results
}

/***********************************
 sound_play
 Input: sound filename
 Output:
 ***********************************/
static int l_sound_play(lua_State* luaState)
{
	const char * l_FileName;
	l_FileName = luaL_checkstring(luaState, -1);
	sfx_play(*(context_get_player()->getConnection()), std::string(l_FileName), ANY_CHANNEL, NO_LOOP);

	return 0; // number of results
}

/***********************************
 get_tick

 Get application tick in milliseconds

 Input:
 Output:
 - tick in milliseconds
 ***********************************/
static int l_get_tick(lua_State* luaState)
{
	Uint32 l_CurrentTime;
	l_CurrentTime = sdl_get_global_time();

	lua_pushnumber(luaState, l_CurrentTime);
	return 1; // number of results
}

/***********************************
 print_text_debug

 Print a message in the client's log (mainly for debug purpose)

 Input:
 - message
 Output:
 ***********************************/
static int l_print_text_debug(lua_State* luaState)
{
	const char * l_pString;

	l_pString = luaL_checkstring(luaState, -1);
	wlog(LOGDESIGNER, (char* ) l_pString);
	return 0;  // number of results
}

/***********************************
 ***********************************/
static void register_lua_functions_client(lua_State * l_pLuaVm)
{
	// player function
	lua_pushcfunction(l_pLuaVm, l_player_get_id);
	lua_setglobal(l_pLuaVm, "player_get_id");
	// context function
	lua_pushcfunction(l_pLuaVm, l_context_get_id);
	lua_setglobal(l_pLuaVm, "context_get_id");
	lua_pushcfunction(l_pLuaVm, l_context_get_npc);
	lua_setglobal(l_pLuaVm, "context_get_npc");
	lua_pushcfunction(l_pLuaVm, l_context_get_map);
	lua_setglobal(l_pLuaVm, "context_get_map");
	// item function
	lua_pushcfunction(l_pLuaVm, l_item_set_px);
	lua_setglobal(l_pLuaVm, "item_set_px");
	lua_pushcfunction(l_pLuaVm, l_item_set_py);
	lua_setglobal(l_pLuaVm, "item_set_py");
	lua_pushcfunction(l_pLuaVm, l_item_get_px);
	lua_setglobal(l_pLuaVm, "item_get_px");
	lua_pushcfunction(l_pLuaVm, l_item_get_py);
	lua_setglobal(l_pLuaVm, "item_get_py");
	lua_pushcfunction(l_pLuaVm, l_item_get_w);
	lua_setglobal(l_pLuaVm, "item_get_w");
	lua_pushcfunction(l_pLuaVm, l_item_get_h);
	lua_setglobal(l_pLuaVm, "item_get_h");
	lua_pushcfunction(l_pLuaVm, l_item_set_anim_start_tick);
	lua_setglobal(l_pLuaVm, "item_set_anim_start_tick");
	lua_pushcfunction(l_pLuaVm, l_item_set_anim);
	lua_setglobal(l_pLuaVm, "item_set_anim");
	lua_pushcfunction(l_pLuaVm, l_item_set_anim_from_context);
	lua_setglobal(l_pLuaVm, "item_set_anim_from_context");
	// camera function
	lua_pushcfunction(l_pLuaVm, l_camera_get_screen);
	lua_setglobal(l_pLuaVm, "camera_get_screen");
	lua_pushcfunction(l_pLuaVm, l_camera_get_zoom);
	lua_setglobal(l_pLuaVm, "camera_get_zoom");
	lua_pushcfunction(l_pLuaVm, l_camera_get_X);
	lua_setglobal(l_pLuaVm, "camera_get_X");
	lua_pushcfunction(l_pLuaVm, l_camera_get_Y);
	lua_setglobal(l_pLuaVm, "camera_get_Y");
	lua_pushcfunction(l_pLuaVm, l_camera_set_zoom);
	lua_setglobal(l_pLuaVm, "camera_set_zoom");
	lua_pushcfunction(l_pLuaVm, l_camera_set_coord);
	lua_setglobal(l_pLuaVm, "camera_set_coord");
	// sound  function
	lua_pushcfunction(l_pLuaVm, l_sound_play);
	lua_setglobal(l_pLuaVm, "sound_play");
	// utility  function
	lua_pushcfunction(l_pLuaVm, l_get_tick);
	lua_setglobal(l_pLuaVm, "get_tick");
	// debug function
	lua_pushcfunction(l_pLuaVm, l_print_text_debug);
	lua_setglobal(l_pLuaVm, "print_text_debug");
}

/***********************************
 ***********************************/
void lua_init()
{
	luaVm = lua_open();
	lua_baselibopen(luaVm);
	lua_tablibopen(luaVm);
	lua_iolibopen(luaVm);
	lua_strlibopen(luaVm);
	lua_mathlibopen(luaVm);
	register_lua_functions_client(luaVm);

	effectLuaVm = lua_open();
	lua_baselibopen(effectLuaVm);
	lua_tablibopen(effectLuaVm);
	lua_iolibopen(effectLuaVm);
	lua_strlibopen(effectLuaVm);
	lua_mathlibopen(effectLuaVm);
	register_lua_functions_client(effectLuaVm);
}

/*****************************************************************************/
lua_State * getLuaVm()
{
	return luaVm;
}

/*****************************************************************************/
SDL_mutex * getLuaVmMutex()
{
	return luaVmMutex;
}

/*****************************************************************************/
lua_State * getEffectLuaVm()
{
	return effectLuaVm;
}

/*****************************************************************************/
SDL_mutex * getEffectLuaVmMutex()
{
	return effectLuaVmMutex;
}
