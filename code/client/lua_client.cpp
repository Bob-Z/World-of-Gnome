/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2017-2019 carabobz@gmail.com

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

#include "Camera.h"
#include "common.h"
#include "Context.h"
#include "imageDB.h"
#include "item.h"
#include "sdl.h"
#include "syntax.h"
#include "entry.h"
#include "log.h"
#include "util.h"
#include "sfx.h"

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

static lua_State * g_pLuaVm = nullptr;
static lua_State * g_pEffectLuaVm = nullptr;

/***********************************
 player_get_id
 Input:
 Output: ID of the current context
 ***********************************/
static int l_player_get_id(lua_State* p_pLuaState)
{
	Context * context;

	context = context_get_player();
	lua_pushstring(p_pLuaState, context->getId().c_str());
	return 1;  // number of results
}

/***********************************
 context_get_id
 Input:
 Output: ID of current context
 ***********************************/
static int l_context_get_id(lua_State* p_pLuaState)
{
	Context * context;

	lua_getglobal(p_pLuaState, "current_context");
	context = (Context*) lua_touserdata(p_pLuaState, -1);
	lua_pop(p_pLuaState, 1);

	lua_pushstring(p_pLuaState, context->getId().c_str());
	return 1; // number of results
}

/***********************************
 context_get_npc
 Input:
 Output: 1 if context is an NPC
 ***********************************/
static int l_context_get_npc(lua_State* p_pLuaState)
{
	Context * context;

	lua_getglobal(p_pLuaState, "current_context");
	context = (Context*) lua_touserdata(p_pLuaState, -1);
	lua_pop(p_pLuaState, 1);

	lua_pushnumber(p_pLuaState, context->isNpc());
	return 1; // number of results
}

/***********************************
 context_get_map
 Input:
 Output: current map name
 ***********************************/
static int l_context_get_map(lua_State* p_pLuaState)
{
	Context * context;

	lua_getglobal(p_pLuaState, "current_context");
	context = (Context*) lua_touserdata(p_pLuaState, -1);
	lua_pop(p_pLuaState, 1);

	lua_pushstring(p_pLuaState, context->getMap().c_str());
	return 1; // number of results
}

/***********************************
 item_set_px
 Input: X ccordinate in pixel
 Output:
 ***********************************/
static int l_item_set_px(lua_State* p_pLuaState)
{
	item_t * l_pItem;

	lua_getglobal(p_pLuaState, "current_item");
	l_pItem = (item_t*) lua_touserdata(p_pLuaState, -1);
	lua_pop(p_pLuaState, 1);

	int l_X;
	l_X = luaL_checkint(p_pLuaState, -1);
	l_pItem->rect.x = l_X;
	return 0; // number of results
}

/***********************************
 item_set_py
 Input: Y ccordinate in pixel
 Output:
 ***********************************/
static int l_item_set_py(lua_State* p_pLuaState)
{
	item_t * l_pItem;

	lua_getglobal(p_pLuaState, "current_item");
	l_pItem = (item_t*) lua_touserdata(p_pLuaState, -1);
	lua_pop(p_pLuaState, 1);

	int l_Y;
	l_Y = luaL_checkint(p_pLuaState, -1);
	l_pItem->rect.y = l_Y;
	return 0; // number of results
}

/***********************************
 item_get_px
 Input:
 Output: X ccordinate in pixel
 ***********************************/
static int l_item_get_px(lua_State* p_pLuaState)
{
	item_t * l_pItem;

	lua_getglobal(p_pLuaState, "current_item");
	l_pItem = (item_t*) lua_touserdata(p_pLuaState, -1);
	lua_pop(p_pLuaState, 1);

	lua_pushnumber(p_pLuaState, l_pItem->rect.x);
	return 1; // number of results
}

/***********************************
 item_get_py
 Input:
 Output: Y ccordinate in pixel
 ***********************************/
static int l_item_get_py(lua_State* p_pLuaState)
{
	item_t * l_pItem;

	lua_getglobal(p_pLuaState, "current_item");
	l_pItem = (item_t*) lua_touserdata(p_pLuaState, -1);
	lua_pop(p_pLuaState, 1);

	lua_pushnumber(p_pLuaState, l_pItem->rect.y);
	return 1; // number of results
}

/***********************************
 item_get_w
 Input:
 Output: Width in pixel
 ***********************************/
static int l_item_get_w(lua_State* p_pLuaState)
{
	item_t * l_pItem;

	lua_getglobal(p_pLuaState, "current_item");
	l_pItem = (item_t*) lua_touserdata(p_pLuaState, -1);
	lua_pop(p_pLuaState, 1);

	lua_pushnumber(p_pLuaState, l_pItem->rect.w);
	return 1; // number of results
}

/***********************************
 item_get_h
 Input:
 Output: Height in pixel
 ***********************************/
static int l_item_get_h(lua_State* p_pLuaState)
{
	item_t * l_pItem;

	lua_getglobal(p_pLuaState, "current_item");
	l_pItem = (item_t*) lua_touserdata(p_pLuaState, -1);
	lua_pop(p_pLuaState, 1);

	lua_pushnumber(p_pLuaState, l_pItem->rect.h);
	return 1; // number of results
}
/***********************************
 item_set_anim_start_tick
 Input: Tick from when animation will be calculated
 Output:
 ***********************************/
static int l_item_set_anim_start_tick(lua_State* p_pLuaState)
{
	item_t * l_pItem;

	lua_getglobal(p_pLuaState, "current_item");
	l_pItem = (item_t*) lua_touserdata(p_pLuaState, -1);
	lua_pop(p_pLuaState, 1);

	int l_Tick;
	l_Tick = luaL_checkint(p_pLuaState, -1);
	l_pItem->anim_start_tick = l_Tick;

	return 0; // number of results
}

/***********************************
 item_set_anim_from a file name
 Input:
 - file name
 Output:
 ***********************************/
static int l_item_set_anim(lua_State* p_pLuaState)
{
	item_t * l_pItem;
	lua_getglobal(p_pLuaState, "current_item");
	l_pItem = (item_t*) lua_touserdata(p_pLuaState, -1);
	lua_pop(p_pLuaState, 1);

	const char * l_pFileName;
	l_pFileName = luaL_checkstring(p_pLuaState, -1);

	anim_t ** l_pAnimArray;

	const char * l_pSpriteNameArray[2] =
	{ l_pFileName, nullptr };
	l_pAnimArray = imageDB_get_anim_array(context_get_player(), (const char **) l_pSpriteNameArray);
	item_set_anim_array(l_pItem, l_pAnimArray);
	return 0; // number of results
}

/***********************************
 ***********************************/
static const char * getKey(int p_IsMoving, char p_Orientation)
{
	const char * l_pKey;

	if (p_IsMoving == true)
	{
		switch (p_Orientation)
		{
		case 'N':
			l_pKey = CHARACTER_KEY_MOV_N_SPRITE;
			break;
		case 'S':
			l_pKey = CHARACTER_KEY_MOV_S_SPRITE;
			break;
		case 'W':
			l_pKey = CHARACTER_KEY_MOV_W_SPRITE;
			break;
		case 'E':
			l_pKey = CHARACTER_KEY_MOV_E_SPRITE;
			break;
		default:
			werr(LOGDESIGNER, "l_item_set_anim_from_context: wrong main orientation");
			l_pKey = CHARACTER_KEY_MOV_S_SPRITE;
			break;
		}
	}
	else
	{
		switch (p_Orientation)
		{
		case 'N':
			l_pKey = CHARACTER_KEY_DIR_N_SPRITE;
			break;
		case 'S':
			l_pKey = CHARACTER_KEY_DIR_S_SPRITE;
			break;
		case 'W':
			l_pKey = CHARACTER_KEY_DIR_W_SPRITE;
			break;
		case 'E':
			l_pKey = CHARACTER_KEY_DIR_E_SPRITE;
			break;
		default:
			werr(LOGDESIGNER, "l_item_set_anim_from_context: wrong main orientation");
			l_pKey = CHARACTER_KEY_DIR_S_SPRITE;
			break;
		}
	}

	return l_pKey;
}

/***********************************
 ***********************************/
static anim_t ** getAnimArray(const char * p_pId, const char * p_pKey)
{
	anim_t ** l_pAnimArray;

	// Try single image anim
	char * sprite_name = nullptr;
	if (entry_read_string(CHARACTER_TABLE, p_pId, &sprite_name, p_pKey, nullptr) == RET_OK)
	{
		if (sprite_name[0] != 0)
		{
			char * l_pSpriteNameArray[2] =
			{ nullptr, nullptr };
			l_pSpriteNameArray[0] = sprite_name;
			l_pAnimArray = imageDB_get_anim_array(context_get_player(), (const char **) l_pSpriteNameArray);
			free(sprite_name);
			return l_pAnimArray;
		}
		free(sprite_name);
	}

	// Try list of image
	char ** sprite_list = nullptr;
	if (entry_read_list(CHARACTER_TABLE, p_pId, &sprite_list, p_pKey, nullptr) == RET_OK)
	{
		l_pAnimArray = imageDB_get_anim_array(context_get_player(), (const char **) sprite_list);
		deep_free(sprite_list);
		return l_pAnimArray;
	}

	return nullptr;
}

/***********************************
 ***********************************/
static char flip(char p_Orientation)
{
	char l_FlipOrientation = 'S';

	switch (p_Orientation)
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

/***********************************
 item_set_anim_from_context
 Input:
 - ID of context
 - entry name in context file
 Output:
 ***********************************/
static int l_item_set_anim_from_context(lua_State* p_pLuaState)
{
	item_t * l_pItem;
	lua_getglobal(p_pLuaState, "current_item");
	l_pItem = (item_t*) lua_touserdata(p_pLuaState, -1);
	lua_pop(p_pLuaState, 1);

	const char * l_pId = "";
	l_pId = luaL_checkstring(p_pLuaState, -4);
	int l_IsMoving = 0;
	l_IsMoving = luaL_checkint(p_pLuaState, -3);
	const char * l_pMainOrientation = "";
	l_pMainOrientation = luaL_checkstring(p_pLuaState, -2);
	const char * l_pSecondaryOrientation = "";
	l_pSecondaryOrientation = luaL_checkstring(p_pLuaState, -1);

	// reset previous anim
	l_pItem->anim.list = nullptr;
	l_pItem->anim.num = 0;

	// Try regular moving flag with main orientation
	const char * l_pKey = getKey(l_IsMoving, l_pMainOrientation[0]);
	anim_t ** l_pAnimArray = getAnimArray(l_pId, l_pKey);

	// Try secondary orientation
	if (l_pAnimArray == nullptr)
	{
		l_pKey = getKey(l_IsMoving, l_pSecondaryOrientation[0]);
		l_pAnimArray = getAnimArray(l_pId, l_pKey);
	}

	// Auto-flip
	if (l_pAnimArray == nullptr)
	{
		char l_FlipOrientation = flip(l_pMainOrientation[0]);
		const char * l_pKey = getKey(l_IsMoving, l_FlipOrientation);
		l_pAnimArray = getAnimArray(l_pId, l_pKey);
	}

	if (l_pAnimArray == nullptr)
	{
		char l_FlipOrientation = flip(l_pSecondaryOrientation[0]);
		l_pKey = getKey(l_IsMoving, l_FlipOrientation);
		l_pAnimArray = getAnimArray(l_pId, l_pKey);
	}

	// Try opposite moving flag
	// main orientation
	if (l_pAnimArray == nullptr)
	{
		l_IsMoving = !l_IsMoving;
		l_pKey = getKey(l_IsMoving, l_pMainOrientation[0]);
		l_pAnimArray = getAnimArray(l_pId, l_pKey);
	}
	// secondary orientation
	if (l_pAnimArray == nullptr)
	{
		l_pKey = getKey(l_IsMoving, l_pSecondaryOrientation[0]);
		l_pAnimArray = getAnimArray(l_pId, l_pKey);
	}

	// Try default sprite
	if (l_pAnimArray == nullptr)
	{
		l_pAnimArray = getAnimArray(l_pId, CHARACTER_KEY_SPRITE);
	}

	if (l_pAnimArray == nullptr)
	{
		werr(LOGDESIGNER, "LUA item_set_anim_from_context: Failed to find anim for %s", l_pId);
	}
	else
	{
		item_set_anim_array(l_pItem, l_pAnimArray);
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
static int l_camera_get_screen(lua_State* p_pLuaState)
{
	Camera * l_pCamera;
	lua_getglobal(p_pLuaState, "current_camera");
	l_pCamera = (Camera*) lua_touserdata(p_pLuaState, -1);
	lua_pop(p_pLuaState, 1);

	lua_pushnumber(p_pLuaState, (int) l_pCamera->getScreen());
	return 1; // number of results
}

/***********************************
 camera_get_zoom

 Get current camera zoom

 Input:
 Output:
 - zoom factor ( +1 for one level of zoom, -1 for UNzoom)
 ***********************************/
static int l_camera_get_zoom(lua_State* p_pLuaState)
{
	Camera * l_pCamera;
	lua_getglobal(p_pLuaState, "current_camera");
	l_pCamera = (Camera*) lua_touserdata(p_pLuaState, -1);
	lua_pop(p_pLuaState, 1);

	lua_pushnumber(p_pLuaState, l_pCamera->getZoom());
	return 1; // number of results
}

/***********************************
 camera_get_X

 Get current camera X position

 Input:
 Output:
 - X position in pixel
 ***********************************/
static int l_camera_get_X(lua_State* p_pLuaState)
{
	Camera * l_pCamera;
	lua_getglobal(p_pLuaState, "current_camera");
	l_pCamera = (Camera*) lua_touserdata(p_pLuaState, -1);
	lua_pop(p_pLuaState, 1);

	lua_pushnumber(p_pLuaState, l_pCamera->getX());
	return 1; // number of results
}

/***********************************
 camera_get_Y

 Get current camera Y position

 Input:
 Output:
 - Y position in pixel
 ***********************************/
static int l_camera_get_Y(lua_State* p_pLuaState)
{
	Camera * l_pCamera;
	lua_getglobal(p_pLuaState, "current_camera");
	l_pCamera = (Camera*) lua_touserdata(p_pLuaState, -1);
	lua_pop(p_pLuaState, 1);

	lua_pushnumber(p_pLuaState, l_pCamera->getY());
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
static int l_camera_set_coord(lua_State* p_pLuaState)
{
	int l_X;
	l_X = luaL_checknumber(p_pLuaState, -2);
	int l_Y;
	l_Y = luaL_checknumber(p_pLuaState, -1);

	sdl_force_virtual_x(l_X);
	sdl_force_virtual_y(l_Y);

	return 0; // number of results
}

/***********************************
 camera_set_zoom

 Set camera zoom

 Input:
 - Zoom level (1.0 = 100%)
 Output:
 ***********************************/
static int l_camera_set_zoom(lua_State* p_pLuaState)
{
	double l_Zoom;
	l_Zoom = luaL_checknumber(p_pLuaState, -1);

	sdl_force_virtual_z(l_Zoom);

	return 0; // number of results
}

/***********************************
 sound_play
 Input: sound filename
 Output:
 ***********************************/
static int l_sound_play(lua_State* p_pLuaState)
{
	const char * l_FileName;
	l_FileName = luaL_checkstring(p_pLuaState, -1);
	sfx_play(context_get_player(), std::string(l_FileName), ANY_CHANNEL, NO_LOOP);

	return 0; // number of results
}

/***********************************
 get_tick

 Get application tick in milliseconds

 Input:
 Output:
 - tick in milliseconds
 ***********************************/
static int l_get_tick(lua_State* p_pLuaState)
{
	Uint32 l_CurrentTime;
	l_CurrentTime = sdl_get_global_time();

	lua_pushnumber(p_pLuaState, l_CurrentTime);
	return 1; // number of results
}

/***********************************
 print_text_debug

 Print a message in the client's log (mainly for debug purpose)

 Input:
 - message
 Output:
 ***********************************/
static int l_print_text_debug(lua_State* p_pLuaState)
{
	const char * l_pString;

	l_pString = luaL_checkstring(p_pLuaState, -1);
	wlog(LOGDESIGNER, (char* ) l_pString);
	return 0;  // number of results
}

/***********************************
 ***********************************/
static void register_lua_functions(lua_State * l_pLuaVm)
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
	g_pLuaVm = lua_open();
	lua_baselibopen(g_pLuaVm);
	lua_tablibopen(g_pLuaVm);
	lua_iolibopen(g_pLuaVm);
	lua_strlibopen(g_pLuaVm);
	lua_mathlibopen(g_pLuaVm);
	register_lua_functions(g_pLuaVm);

	g_pEffectLuaVm = lua_open();
	lua_baselibopen(g_pEffectLuaVm);
	lua_tablibopen(g_pEffectLuaVm);
	lua_iolibopen(g_pEffectLuaVm);
	lua_strlibopen(g_pEffectLuaVm);
	lua_mathlibopen(g_pEffectLuaVm);
	register_lua_functions(g_pEffectLuaVm);
}

/***********************************
 ***********************************/
lua_State * getLuaVm()
{
	return g_pLuaVm;
}

/***********************************
 ***********************************/
lua_State * getEffectLuaVm()
{
	return g_pEffectLuaVm;
}
