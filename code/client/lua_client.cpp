/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2017 carabobz@gmail.com

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

#include "context.h"
#include "imageDB.h"
#include "../common/common.h"
#include "../sdl_item/sdl.h"
#include "../sdl_item/item.h"
#include "Camera.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#ifdef __cplusplus
}
#endif

static lua_State * luaVM = nullptr;

/***********************************
 player_get_id
Input:
Output: ID of the current context
***********************************/
static int l_player_get_id(lua_State* p_pLuaState)
{
        context_t * l_pContext;

	l_pContext = context_get_player();
        lua_pushstring(p_pLuaState, l_pContext->id);
        return 1;  // number of results
}

/***********************************
 context_get_id
Input:
Output: ID of current context
***********************************/
static int l_context_get_id(lua_State* p_pLuaState)
{
        context_t * l_pContext;

        lua_getglobal(p_pLuaState,"current_context");
        l_pContext = (context_t*)lua_touserdata(p_pLuaState, -1);
        lua_pop(p_pLuaState,1);

	lua_pushstring(p_pLuaState, l_pContext->id);
	return 1; // number of results
}

/***********************************
 context_get_npc
Input:
Output: 1 if context is an NPC
***********************************/
static int l_context_get_npc( lua_State* p_pLuaState)
{
        context_t * l_pContext;

        lua_getglobal(p_pLuaState,"current_context");
        l_pContext = (context_t*)lua_touserdata(p_pLuaState, -1);
        lua_pop(p_pLuaState,1);

	lua_pushnumber(p_pLuaState, l_pContext->npc);
	return 1; // number of results
}

/***********************************
 context_get_map
Input:
Output: current map name
***********************************/
static int l_context_get_map( lua_State* p_pLuaState)
{
        context_t * l_pContext;

        lua_getglobal(p_pLuaState,"current_context");
        l_pContext = (context_t*)lua_touserdata(p_pLuaState, -1);
        lua_pop(p_pLuaState,1);

	lua_pushstring(p_pLuaState, l_pContext->map);
	return 1; // number of results
}

/***********************************
 item_set_x
Input: X ccordinate in pixel
Output:
***********************************/
static int l_item_set_x( lua_State* p_pLuaState)
{
        item_t * l_pItem;

        lua_getglobal(p_pLuaState,"current_item");
        l_pItem = (item_t*)lua_touserdata(p_pLuaState, -1);
        lua_pop(p_pLuaState,1);

	int l_X;
	l_X = luaL_checkint(p_pLuaState, -1);
	l_pItem->rect.x = l_X;
	return 0; // number of results
}

/***********************************
 item_set_y
Input: Y ccordinate in pixel
Output:
***********************************/
static int l_item_set_y( lua_State* p_pLuaState)
{
        item_t * l_pItem;

        lua_getglobal(p_pLuaState,"current_item");
        l_pItem = (item_t*)lua_touserdata(p_pLuaState, -1);
        lua_pop(p_pLuaState,1);

	int l_Y;
	l_Y = luaL_checkint(p_pLuaState, -1);
	l_pItem->rect.y = l_Y;
	return 0; // number of results
}

/***********************************
 item_get_x
Input:
Output: X ccordinate in pixel
***********************************/
static int l_item_get_x( lua_State* p_pLuaState)
{
        item_t * l_pItem;

        lua_getglobal(p_pLuaState,"current_item");
        l_pItem = (item_t*)lua_touserdata(p_pLuaState, -1);
        lua_pop(p_pLuaState,1);

	lua_pushnumber(p_pLuaState, l_pItem->rect.x);
	return 1; // number of results
}

/***********************************
 item_get_y
Input:
Output: Y ccordinate in pixel
***********************************/
static int l_item_get_y( lua_State* p_pLuaState)
{
        item_t * l_pItem;

        lua_getglobal(p_pLuaState,"current_item");
        l_pItem = (item_t*)lua_touserdata(p_pLuaState, -1);
        lua_pop(p_pLuaState,1);

	lua_pushnumber(p_pLuaState, l_pItem->rect.y);
	return 1; // number of results
}

/***********************************
 item_set_anim_start_tick
Input: Tick from when animation will be calculated
Output: 
***********************************/
static int l_item_set_anim_start_tick( lua_State* p_pLuaState)
{
        item_t * l_pItem;

        lua_getglobal(p_pLuaState,"current_item");
        l_pItem = (item_t*)lua_touserdata(p_pLuaState, -1);
        lua_pop(p_pLuaState,1);

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
static int l_item_set_anim( lua_State* p_pLuaState)
{
        item_t * l_pItem;
        lua_getglobal(p_pLuaState,"current_item");
        l_pItem = (item_t*)lua_touserdata(p_pLuaState, -1);
        lua_pop(p_pLuaState,1);

	const char * l_pFileName;
        l_pFileName = luaL_checkstring(p_pLuaState, -1);

	anim_t ** l_pAnimArray;

	const char * l_pSpriteNameArray[2] = { l_pFileName, nullptr };
	l_pAnimArray = imageDB_get_anim_array(context_get_player(),(const char **)l_pSpriteNameArray);
	item_set_anim_array(l_pItem,l_pAnimArray);
	return 0; // number of results
}

/***********************************
 item_set_anim_from_context
Input:
 - ID of context
 - entry name in context file
Output:
***********************************/
static int l_item_set_anim_from_context( lua_State* p_pLuaState)
{
        item_t * l_pItem;
        lua_getglobal(p_pLuaState,"current_item");
        l_pItem = (item_t*)lua_touserdata(p_pLuaState, -1);
        lua_pop(p_pLuaState,1);

	const char * l_pId;
        l_pId = luaL_checkstring(p_pLuaState, -2);
	const char * l_pEntryName;
        l_pEntryName = luaL_checkstring(p_pLuaState, -1);

	anim_t ** l_pAnimArray;

	char * sprite_name = nullptr;
	if( entry_read_string(CHARACTER_TABLE,l_pId,&sprite_name,l_pEntryName,nullptr) == RET_OK) {
		if(sprite_name[0] != 0) {
			char * l_pSpriteNameArray[2] = { nullptr, nullptr };
			l_pSpriteNameArray[0] = sprite_name;
			l_pAnimArray = imageDB_get_anim_array(context_get_player(),(const char **)l_pSpriteNameArray);
			free(sprite_name);

			item_set_anim_array(l_pItem,l_pAnimArray);
			return 0;
		}
		free(sprite_name);
	}

	char ** sprite_list = nullptr;
	if( entry_read_list(CHARACTER_TABLE,l_pId,&sprite_list,l_pEntryName,nullptr) == RET_OK ) {
		l_pAnimArray = imageDB_get_anim_array(context_get_player(),(const char **)sprite_list);
                deep_free(sprite_list);

		item_set_anim_array(l_pItem,l_pAnimArray);
		return 0;
	}

	char l_pErrorMessage[1024];
	snprintf(l_pErrorMessage,sizeof(l_pErrorMessage),"LUA item_set_anim_from_context: Failed to get %s in %s", l_pEntryName, l_pId);
	werr(LOGDEV,l_pErrorMessage);

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
        lua_getglobal(p_pLuaState,"current_camera");
        l_pCamera = (Camera*)lua_touserdata(p_pLuaState, -1);
        lua_pop(p_pLuaState,1);

	lua_pushnumber(p_pLuaState, (int)l_pCamera->getScreen());
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
        lua_getglobal(p_pLuaState,"current_camera");
        l_pCamera = (Camera*)lua_touserdata(p_pLuaState, -1);
        lua_pop(p_pLuaState,1);

	lua_pushnumber(p_pLuaState, l_pCamera->getZoom());
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
        wlog(LOGDEV,(char*)l_pString);
        return 0;  // number of results
}

/***********************************
***********************************/
static void register_lua_functions()
{
	// player func
	lua_pushcfunction(luaVM, l_player_get_id);
	lua_setglobal(luaVM, "player_get_id");
	// context func
	lua_pushcfunction(luaVM, l_context_get_id);
	lua_setglobal(luaVM, "context_get_id");
	lua_pushcfunction(luaVM, l_context_get_npc);
	lua_setglobal(luaVM, "context_get_npc");
	lua_pushcfunction(luaVM, l_context_get_map);
	lua_setglobal(luaVM, "context_get_map");
	// item func
	lua_pushcfunction(luaVM, l_item_set_x);
	lua_setglobal(luaVM, "item_set_x");
	lua_pushcfunction(luaVM, l_item_set_y);
	lua_setglobal(luaVM, "item_set_y");
	lua_pushcfunction(luaVM, l_item_get_x);
	lua_setglobal(luaVM, "item_get_x");
	lua_pushcfunction(luaVM, l_item_get_y);
	lua_setglobal(luaVM, "item_get_y");
	lua_pushcfunction(luaVM, l_item_set_anim_start_tick);
	lua_setglobal(luaVM, "item_set_anim_start_tick");
	lua_pushcfunction(luaVM, l_item_set_anim);
	lua_setglobal(luaVM, "item_set_anim");
	lua_pushcfunction(luaVM, l_item_set_anim_from_context);
	lua_setglobal(luaVM, "item_set_anim_from_context");
	// camera func
	lua_pushcfunction(luaVM, l_camera_get_screen);
	lua_setglobal(luaVM, "camera_get_screen");
	lua_pushcfunction(luaVM, l_camera_get_zoom);
	lua_setglobal(luaVM, "camera_get_zoom");
	lua_pushcfunction(luaVM, l_camera_set_zoom);
	lua_setglobal(luaVM, "camera_set_zoom");
	lua_pushcfunction(luaVM, l_camera_set_coord);
	lua_setglobal(luaVM, "camera_set_coord");
	// utility  func
	lua_pushcfunction(luaVM, l_get_tick);
	lua_setglobal(luaVM, "get_tick");
	// debug func
	lua_pushcfunction(luaVM, l_print_text_debug);
	lua_setglobal(luaVM, "print_text_debug");
}

/***********************************
***********************************/
void lua_init()
{
	luaVM = lua_open();
        lua_baselibopen(luaVM);
        lua_tablibopen(luaVM);
        lua_iolibopen(luaVM);
        lua_strlibopen(luaVM);
        lua_mathlibopen(luaVM);

        register_lua_functions();
}

/***********************************
***********************************/
lua_State * get_luaVM()
{
	return luaVM;
}

