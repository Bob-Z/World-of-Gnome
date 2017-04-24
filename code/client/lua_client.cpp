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
#include "../common/common.h"
#include "../sdl_item/item.h"

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
static int l_player_get_id( lua_State* L)
{
        context_t * context;

	context = context_get_player();
        lua_pushstring(L, context->id);
        return 1;  // number of results
}

/***********************************
 item set_x
Input: X ccordinate in pixel
Output:
***********************************/
static int l_item_set_x( lua_State* L)
{
        item_t * item;

        lua_getglobal(L,"current_item");
        item = (item_t*)lua_touserdata(L, -1);
        lua_pop(L,1);

	int l_X;
	l_X = luaL_checkint(L, -1);
	item->rect.x = l_X;
	return 0; // number of results
}

/***********************************
 print_text_debug

Print a message in the client's log (mainly for debug purpose)

Input:
 - message
Output:
***********************************/
static int l_print_text_debug( lua_State* L)
{
        const char * string;

        string = luaL_checkstring(L, -1);
        wlog(LOGDEV,(char*)string);
        return 0;  // number of results
}

/***********************************
***********************************/
static void register_lua_functions()
{
	// player func
	lua_pushcfunction(luaVM, l_player_get_id);
	lua_setglobal(luaVM, "player_get_id");
	// item func
	lua_pushcfunction(luaVM, l_item_set_x);
	lua_setglobal(luaVM, "item_set_x");
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

