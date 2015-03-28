/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013-2015 carabobz@gmail.com

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

#include "../common/common.h"
#include "character.h"
#include "equipment.h"
#include "inventory.h"
#include "attribute.h"
#include "network_server.h"
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>

#define LUAVM_CONTEXT "wog_context"

/* LUA script functions */

/* player_get_id
Input:
Output: ID of the current context
*/
static int l_player_get_id( lua_State* L)
{
	context_t * context;

	lua_getglobal(L,LUAVM_CONTEXT);
	context = lua_touserdata(L, -1);
	lua_pop(L,1);
	lua_pushstring(L, context->id);
	return 1;  /* number of results */
}

/* character_create_from_template
Input:
 - character template name
 - map of the newly created character
 - layer of the newly created character
 - x ...
 - y ...
Output:
	id of the new character or nil if cannot be created or placed
*/
static int l_character_create_from_template( lua_State* L)
{
	const char * template;
	const char * map;
	int layer;
	int x;
	int y;

	char * res;
	context_t * ctx;

	lua_getglobal(L,LUAVM_CONTEXT);
	ctx = lua_touserdata(L, -1);
	lua_pop(L,1);

	template = luaL_checkstring(L, -4);
	map = luaL_checkstring(L, -4);
	layer = luaL_checkint(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	res = character_create_from_template(ctx,template,map,layer,x,y);
	lua_pushstring(L, res);
	if( res) {
		free(res);
	}
	return 1;  /* number of results */
}

/* character_get_selected_map_tile_x
Input:
 - ID of a character
Output: X coordinate if selected tile
*/
static int l_character_get_selected_map_tile_x( lua_State* L)
{
	context_t * target;
	const char * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		werr(LOGDEV,"Cannot find context with ID %s",id);
		return 0;  /* number of results */
	}
	lua_pushnumber(L, target->selection.map_coord[0]);
	return 1;  /* number of results */
}

/* character_get_selected_map_tile_y
Input:
 - ID of a character
Output: Y coordinate if selected tile
*/
static int l_character_get_selected_map_tile_y( lua_State* L)
{
	context_t * target;
	const char * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		werr(LOGDEV,"Cannot find context with ID %s",id);
		return 0;  /* number of results */
	}
	lua_pushnumber(L, target->selection.map_coord[1]);
	return 1;  /* number of results */
}

/* character_get_selected_map
Input:
 - ID of a character
Output: ID of the selected map
*/
static int l_character_get_selected_map( lua_State* L)
{
	context_t * target;
	const char * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		werr(LOGDEV,"Cannot find context with ID %s",id);
		return 0;  /* number of results */
	}
	lua_pushstring(L, target->selection.map);
	return 1;  /* number of results */
}

/* character_set_selected_tile
Input:
 - ID of a character
 - ID of a map
 - X coord of the selected tile in this map
 - Y coord of the selected tile in this map
Output:
*/
static int l_character_set_selected_tile( lua_State* L)
{
	context_t * target;
	const char * id;
	const char * selected_map;
	int x;
	int y;

	id = luaL_checkstring(L, -4);
	selected_map = luaL_checkstring(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);

	target = context_find(id);
	if( target == NULL ) {
		werr(LOGDEV,"Cannot find context with ID %s",id);
		return 0;  /* number of results */
	}

	if( context_set_selected_tile(target,selected_map,x,y) ) {
		network_send_context_to_context(target,target);
	}

	return 0;  /* number of results */
}

/* character_get_selected_inventory_id
Input:
 - ID of a character
Output: ID of selected item in inventory
*/
static int l_character_get_selected_inventory_id( lua_State* L)
{
	context_t * target;
	const char * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		werr(LOGDEV,"Cannot find context with ID %s",id);
		return 0;  /* number of results */
	}
	lua_pushstring(L, target->selection.inventory);
	return 1;  /* number of results */
}

/* character_set_selected_inventory_id
Input:
 - ID of a character
 - ID of an item
Output:
*/
static int l_character_set_selected_inventory_id( lua_State* L)
{
	context_t * target;
	const char * id;
	const char * selected_item;

	id = luaL_checkstring(L, -2);
	selected_item = luaL_checkstring(L, -1);

	target = context_find(id);
	if( target == NULL ) {
		werr(LOGDEV,"Cannot find context with ID %s",id);
		return 0;  /* number of results */
	}

	if( context_set_selected_item(target,selected_item) ) {
		network_send_context_to_context(target,target);
	}

	return 0;  /* number of results */
}

/* character_get_selected_equipment_slot
Input:
 - ID of a character
Output: ID of selected item in equipment
*/
static int l_character_get_selected_equipment_slot( lua_State* L)
{
	context_t * target;
	const char * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		werr(LOGDEV,"Cannot find context with ID %s",id);
		return 0;  /* number of results */
	}
	lua_pushstring(L, target->selection.equipment);
	return 1;  /* number of results */
}

/* character_set_selected_equipment_slot
Input:
 - ID of a character
 - ID of an equipment slot
Output:
*/
static int l_character_set_selected_equipment_slot( lua_State* L)
{
	context_t * target;
	const char * id;
	const char * selected_equipment;

	id = luaL_checkstring(L, -2);
	selected_equipment = luaL_checkstring(L, -1);

	target = context_find(id);
	if( target == NULL ) {
		werr(LOGDEV,"Cannot find context with ID %s",id);
		return 0;  /* number of results */
	}

	if( context_set_selected_equipment(target,selected_equipment) ) {
		network_send_context_to_context(target,target);
	}

	return 0;  /* number of results */
}

/* character_get_selected_character_id
Input:
 - ID of a character
Output: ID of the selected character
*/
static int l_character_get_selected_character_id( lua_State* L)
{
	context_t * target;
	const char * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		werr(LOGDEV,"Cannot find context with ID %s",id);
		return 0;  /* number of results */
	}
	lua_pushstring(L, target->selection.id);
	return 1;  /* number of results */
}

/* character_set_selected_character_id
Input:
 - ID of a character
 - ID of selected character
Output:
*/
static int l_character_set_selected_character_id( lua_State* L)
{
	context_t * target;
	const char * id;
	const char * selected_id;

	id = luaL_checkstring(L, -2);
	selected_id = luaL_checkstring(L, -1);

	target = context_find(id);
	if( target == NULL ) {
		werr(LOGDEV,"Cannot find context with ID %s",id);
		return 0;  /* number of results */
	}

	if( context_set_selected_character(target,selected_id) ) {
		network_send_context_to_context(target,target);
	}

	return 0;  /* number of results */
}

/* character_get_map
Input:
 - ID of a character
Output: ID of the map where the character is
*/
static int l_character_get_map( lua_State* L)
{
	context_t * target;
	const char * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		werr(LOGDEV,"Cannot find context with ID %s",id);
		return 0;  /* number of results */
	}
	lua_pushstring(L, target->map);
	return 1;  /* number of results */
}

/* character_get_map_w
Input:
 - ID of a map
Output: Width of the map
*/
static int l_character_get_map_w( lua_State* L)
{
	context_t * target;
	const char * id;
	int map_w = -1;
	int player_layer = 0;
	char layer_name[SMALL_BUF];

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		werr(LOGDEV,"Cannot find context with ID %s",id);
		return 0;  /* number of results */
	}

	entry_read_int(CHARACTER_TABLE,target->id,&player_layer,CHARACTER_KEY_LAYER,NULL);
	sprintf(layer_name,"%s%d",MAP_KEY_LAYER,player_layer);
	entry_read_int(MAP_TABLE,target->map,&map_w,layer_name,MAP_KEY_WIDTH,NULL);

	lua_pushnumber(L, map_w);
	return 1;  /* number of results */
}

/* character_get_map_h
Input:
 - ID of a map
Output: Height of the map
*/
static int l_character_get_map_h( lua_State* L)
{
	context_t * target;
	const char * id;
	int map_h = -1;
	int player_layer = 0;
	char layer_name[SMALL_BUF];

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		werr(LOGDEV,"Cannot find context with ID %s",id);
		return 0;  /* number of results */
	}

	entry_read_int(CHARACTER_TABLE,target->id,&player_layer,CHARACTER_KEY_LAYER,NULL);
	sprintf(layer_name,"%s%d",MAP_KEY_LAYER,player_layer);
	entry_read_int(MAP_TABLE,target->map,&map_h,layer_name,MAP_KEY_HEIGHT,NULL);

	lua_pushnumber(L, map_h);
	return 1;  /* number of results */
}

/* character_get_x
Input:
 - ID of a character
Output: X coordinate of the character
*/
static int l_character_get_x( lua_State* L)
{
	context_t * target;
	const char * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		werr(LOGDEV,"Cannot find context with ID %s",id);
		return 0;  /* number of results */
	}
	lua_pushnumber(L, target->pos_x);
	return 1;  /* number of results */
}

/* character_get_y
Input:
 - ID of a character
Output: Y coordinate of the character
*/
static int l_character_get_y( lua_State* L)
{
	context_t * target;
	const char * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		werr(LOGDEV,"Cannot find context with ID %s",id);
		return 0;  /* number of results */
	}
	lua_pushnumber(L, target->pos_y);
	return 1;  /* number of results */
}

/* character_get_name
Input:
 - ID of a character
Output: Name of the character
*/
static int l_character_get_name( lua_State* L)
{
	context_t * target;
	const char * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		werr(LOGDEV,"Cannot find context with ID %s",id);
		return 0;  /* number of results */
	}
	lua_pushstring(L, target->character_name);
	return 1;  /* number of results */
}

/* character_get_type
Input:
 - ID of a character
Output: Type of the character
*/
static int l_character_get_type( lua_State* L)
{
	context_t * target;
	const char * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		werr(LOGDEV,"Cannot find context with ID %s",id);
		return 0;  /* number of results */
	}
	lua_pushstring(L, target->type);
	return 1;  /* number of results */
}

/* character_out_of_game

Kick a context out of the game.
This does not disconnect it.

Input:
 - ID of a character
Output:
*/
static int l_character_out_of_game( lua_State* L)
{
	const char * id;
	int res;

	id = luaL_checkstring(L, -1);
	res = character_out_of_game(id);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}
/* character_disconnect

Disconnect a context
This kills a NPC AI

Input:
 - ID of a character
Output:
*/
static int l_character_disconnect( lua_State* L)
{
	const char * id;
	int res;

	id = luaL_checkstring(L, -1);
	res = character_disconnect(id);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* character_delete

Delete the memory and file used by a character

Input:
 - ID of a character
Output:
0 if success
*/
static int l_character_delete( lua_State* L)
{
	const char * id;
	int res;

	id = luaL_checkstring(L, -1);
	res = entry_destroy(CHARACTER_TABLE,id);
	if( res == 0 ) {
		res = file_delete(CHARACTER_TABLE,id);
	}
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* character_set_pos

Set a character's position.
May fail with regard to allowed tiles

Input:
 - ID of a character
 - ID of the map to set
 - layer of the map
 - X cooridnate in the map
 - Y cooridnate in the map
Output:
 return -1 if the position can not be set.
*/
static int l_character_set_pos( lua_State* L)
{
	const char * id;
	const char * map;
	int layer;
	int x;
	int y;
	int res;
	context_t * ctx;

	id = luaL_checkstring(L, -5);
	map = luaL_checkstring(L, -4);
	layer = luaL_checkint(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);

	ctx = context_find(id);
	res = character_set_pos(ctx,map,layer,x,y);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* character_set_npc

Set a character as a non-player character and starts it's AI script.

Input:
 - ID of a character
 - Value of NPC (0 for player's character, 1 for NPC)
Output:
*/
static int l_character_set_npc( lua_State* L)
{
	const char * id;
	int npc;
	int res;

	id = luaL_checkstring(L, -2);
	npc = luaL_checkint(L, -1);

	res = character_set_npc(id,npc);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* character_get_npc

Get a characters npc attribute.

Input:
 - ID of a character
Output:
 - 0 if not NPC
 - 1 if NPC
*/
static int l_character_get_npc( lua_State* L)
{
	const char * id;
	int res;

	id = luaL_checkstring(L, -1);

	res = character_get_npc(id);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* character_get_portrait

Get portrait image file name

Input:
 - ID of a character
Output: portrait file name
*/
static int l_character_get_portrait( lua_State* L)
{
	const char * id;
	char * res;

	id = luaL_checkstring(L, -1);

	res = character_get_portrait(id);
	lua_pushstring(L, res);
	if( res) {
		free(res);
	}
	return 1;  /* number of results */
}

/* character_set_portrait

Set a character's portrait.

Input:
 - ID of a character
 - filename of the portrait
Output:
*/
static int l_character_set_portrait( lua_State* L)
{
	const char * id;
	const char * portrait;
	int res;

	id = luaL_checkstring(L, -2);
	portrait = luaL_checkstring(L, -1);

	res = character_set_portrait(id,portrait);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* map_new

Create a map

Input:
 - Suggested file name (if empty an available name if automatically found)
 - layer to be created
 - Width of map
 - Height of map
 - Width of tile (in pixels)
 - Height of tile (in pixels)
 - default tile (new map is filled with)
 - default type (new map is filled with)
Output: New map ID
*/
static int l_map_new( lua_State* L)
{
	char * map_name;
	const char * suggested_name;
	int layer;
	int x;
	int y;
	int tile_x;
	int tile_y;
	const char * default_tile;
	const char * default_type;

	suggested_name = luaL_checkstring(L, -8);
	layer = luaL_checkint(L, -7);
	x = luaL_checkint(L, -6);
	y = luaL_checkint(L, -5);
	tile_x = luaL_checkint(L, -4);
	tile_y = luaL_checkint(L, -3);
	default_tile = luaL_checkstring(L, -2);
	default_type = luaL_checkstring(L, -1);
	map_name = map_new(suggested_name,layer,x,y,tile_x,tile_y,default_tile,default_type);
	lua_pushstring(L, map_name);
	free(map_name);
	return 1;  /* number of results */
}

/* map_set_tile

Set a tile in a map

Input:
 - ID of a map
 - layer of the map
 - ID of a tile
 - X coordinate of the tile to set
 - Y coordinate of the tile to set
 - Map level
Output:
*/
static int l_map_set_tile( lua_State* L)
{
	const char * map;
	int layer;
	const char * tile;
	int x;
	int y;
	int res;

	map = luaL_checkstring(L, -5);
	layer = luaL_checkint(L, -4);
	tile = luaL_checkstring(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	res = map_set_tile(map, layer, tile, x, y);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* map_set_tile_type

Set a type in a map

Input:
 - ID of a map
 - layer of the map
 - type
 - X coordinate of the tile to set
 - Y coordinate of the tile to set
Output:
*/
static int l_map_set_tile_type( lua_State* L)
{
	const char * map;
	int layer;
	const char * type;
	int x;
	int y;
	int res;

	map = luaL_checkstring(L, -5);
	layer = luaL_checkint(L, -4);
	type = luaL_checkstring(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	res = map_set_tile_type(map,layer,type,x,y);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* print_text_id

Send a message to a character

Input:
 - ID of the receiver
 - message
Output:
*/
static int l_print_text_id( lua_State* L)
{
	const char * id;
	const char * string;

	id = luaL_checkstring(L, -2);
	string = luaL_checkstring(L, -1);
	/* add a trailing \n */
	network_send_text(id,string);
	return 0;  /* number of results */
}

/* print_text_map

Send a message to all characters on a map

Input:
 - ID of a map
 - message
Output:
*/
static int l_print_text_map( lua_State* L)
{
	const char * map;
	const char * string;

	map = luaL_checkstring(L, -2);
	string = luaL_checkstring(L, -1);
	/* add a trailing \n */
	context_broadcast_text(map,string);
	return 0;  /* number of results */
}

/* print_text_server

Send a message to all characters on the server

Input:
 - message
Output:
*/
static int l_print_text_server( lua_State* L)
{
	const char * string;

	string = luaL_checkstring(L, -1);
	/* add a trailing \n */
	context_broadcast_text(NULL,string);
	return 0;  /* number of results */
}

/* print_text_debug

Print a message in the server's log (mainly for debug purpose)

Input:
 - message
Output:
*/
static int l_print_text_debug( lua_State* L)
{
	const char * string;

	string = luaL_checkstring(L, -1);
	wlog(LOGDEV,(char*)string);
	return 0;  /* number of results */
}

/* map_add_item

Add an item on a map

Input:
 - ID of a map
 - Layer of the map
 - ID of an item
 - X coordinate in the map
 - Y coordinate in the map
Output:
*/
static int l_map_add_item( lua_State* L)
{
	const char * map;
	int layer;
	const char * item;
	int x;
	int y;
	int res;

	map = luaL_checkstring(L, -5);
	layer = luaL_checkint(L, -4);
	item = luaL_checkstring(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	res = map_add_item(map,layer,item,x,y);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* map_delete_item

Remove an item from a map (only the first item found on the tile is deleted)

Input:
 - ID of a map
 - Layer of map
 - X coordinate in the map
 - Y coordinate in the map
Output:
*/
static int l_map_delete_item( lua_State* L)
{
	const char * map;
	int layer;
	int x;
	int y;
	char * res;

	map = luaL_checkstring(L, -4);
	layer = luaL_checkint(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	res = map_delete_item(map,layer,x,y);
	lua_pushstring(L, res);
	if(res) {
		free(res);
	}
	return 1;  /* number of results */
}

/* map_add_event

Add an event on a map

Input:
 - ID of a map
 - Layer of the map
 - Event's script
 - X coordinate in the map
 - Y coordinate in the map
Output: Event ID
*/
static int l_map_add_event( lua_State* L)
{
	const char * map;
	int layer;
	const char * script;
	int x;
	int y;
	char *res;

	map = luaL_checkstring(L, -5);
	layer = luaL_checkint(L, -4);
	script = luaL_checkstring(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	res = map_add_event(map,layer,script,x,y);
	lua_pushstring(L, res);
	if(res) {
		free(res);
	}
	return 1;  /* number of results */
}

/* map_add_event_param

Add a parameter to the given event

Input:
 - ID of a map
 - Layer of the map
 - ID of an event
 - parameter to add
Output:
*/
static int l_map_add_event_param( lua_State* L)
{
	const char * map;
	int layer;
	const char * event_id;
	const char * param;
	int res;

	map = luaL_checkstring(L, -4);
	layer = luaL_checkint(L, -3);
	event_id = luaL_checkstring(L, -2);
	param = luaL_checkstring(L, -1);
	res = map_add_event_param(map,layer,event_id,param);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* map_delete_event

Add a parameter to the given event

Input:
 - ID of a map
 - Layer of the map
 - event's script
 - X coordinate (in tiles)
 - Y coordinate (in tiles)
Output:
*/
static int l_map_delete_event( lua_State* L)
{
	const char * map;
	int layer;
	const char * script;
	int x;
	int y;
	int res;

	map = luaL_checkstring(L, -5);
	layer = luaL_checkint(L, -4);
	script = luaL_checkstring(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	res = map_delete_event(map,layer,script,x,y);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* inventory_delete

Delete an item from the inventory

Input:
 - ID of a character
 - ID of an item
Output:
*/
static int l_inventory_delete( lua_State* L)
{
	const char * id;
	const char * item;
	int res;

	id = luaL_checkstring(L, -2);
	item = luaL_checkstring(L, -1);
	res = inventory_delete(id,item);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* inventory_add

Add an item in the inventory

Input:
 - ID of a character
 - ID of an item
Output:
*/
static int l_inventory_add( lua_State* L)
{
	const char * id;
	const char * item;
	int res;

	id = luaL_checkstring(L, -2);
	item = luaL_checkstring(L, -1);
	res = inventory_add(id,item);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* inventory_get_by_name

Get item ID from its name

Input:
 - ID of a character
 - name of an item
Output: ID of an item of that type
*/
static int l_inventory_get_by_name( lua_State* L)
{
	const char * id;
	const char * item_name;
	char * res;

	id = luaL_checkstring(L, -2);
	item_name = luaL_checkstring(L, -1);
	res = inventory_get_by_name(id,item_name);
	lua_pushstring(L, res);
	if( res) {
		free(res);
	}
	return 1;  /* number of results */
}

/* item_create_empty

Create an empty item

Input:
Output: ID of a new item
*/
static int l_item_create_empty( lua_State* L)
{
	char * res;

	res = item_create_empty();
	lua_pushstring(L, res);
	if( res) {
		free(res);
	}
	return 1;  /* number of results */
}

/* item_create_from_template

Create an item from a template

Input: item template name
Output: ID of a new item
*/
static int l_item_create_from_template( lua_State* L)
{
	const char * item;
	char * res;

	item = luaL_checkstring(L, -1);
	res = item_create_from_template(item);
	lua_pushstring(L, res);
	if( res) {
		free(res);
	}
	return 1;  /* number of results */
}

/* resource_new

Create a new resource

Input:
 - Name of the template
 - Quantity
Output: ID of the new resource
*/
static int l_resource_new( lua_State* L)
{
	const char * template;
	int quantity;
	char * resource;

	template = luaL_checkstring(L, -2);
	quantity = luaL_checkint(L, -1);

	resource = resource_new(template,quantity);
	lua_pushstring(L, resource);
	if( resource) {
		free(resource);
	}
	return 1;  /* number of results */
}

/* resource_get_quantity

Get the quantity of a resource

Input:
 - ID of a resource
Output: Quantity of that resource
*/
static int l_resource_get_quantity( lua_State* L)
{
	const char * resource;
	int res;

	resource = luaL_checkstring(L, -1);
	res = resource_get_quantity(resource);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* resource_set_quantity

Set the quantity of a resource

Input:
 - ID of a resource
 - quantity to set
Output: -1 on error
*/
static int l_resource_set_quantity( lua_State* L)
{
	const char * resource;
	int quantity;
	int res;
	context_t * context;

	lua_getglobal(L,LUAVM_CONTEXT);
	context = lua_touserdata(L, -1);
	lua_pop(L,1);

	resource = luaL_checkstring(L, -2);
	quantity = luaL_checkint(L, -1);
	res = resource_set_quantity(context,resource,quantity);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* item_destroy

Delete an item

Input:
 - ID of an item
Output:
*/
static int l_item_destroy( lua_State* L)
{
	const char * item;
	int res;

	item = luaL_checkstring(L, -1);
	res = item_destroy(item);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* map_get_tile

Get tile ID of a given map

Input:
 - ID of a map
 - Layer of the map
 - X coordinate (in tiles)
 - Y coordinate (in tiles)
 - Map level
Output: ID of the tile
*/
static int l_map_get_tile( lua_State* L)
{
	const char * map;
	int layer;
	int x;
	int y;
	char * res;

	map = luaL_checkstring(L, -4);
	layer = luaL_checkint(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	res = map_get_tile(map,layer,x,y);
	lua_pushstring(L, res);
	if( res) {
		free(res);
	}
	return 1;  /* number of results */
}

/* map_get_tile_type

Get tile type of a given map

Input:
 - ID of a map
 - Layer of the map
 - X coordinate (in tiles)
 - Y coordinate (in tiles)
Output: type of the tile
*/
static int l_map_get_tile_type( lua_State* L)
{
	const char * map;
	int layer;
	int x;
	int y;
	char * res;

	map = luaL_checkstring(L, -4);
	layer = luaL_checkint(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	res = map_get_tile_type(map,layer,x,y);
	lua_pushstring(L, res);
	free(res);
	return 1;  /* number of results */
}

/* map_get_character

Get list of characters on a tile

Input:
 - ID of a map
 - Layer of the map
 - X coordinate (in tiles)
 - Y coordinate (in tiles)
Output: Array of characters on that tile
*/
static int l_map_get_character( lua_State* L)
{
	const char * map;
	int layer;
	int x;
	int y;
	char ** res;
	char ** cur_res;
	int res_num = 0;

	map = luaL_checkstring(L, -4);
	layer = luaL_checkint(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);

	res = map_get_character(map,layer,x,y);
	if( res) {
		cur_res = res;
		while(*cur_res != NULL) {
			lua_pushstring(L, *cur_res);
			free(*cur_res);
			res_num++;
			cur_res++;
		}
		free(res);
	}

	return res_num;  /* number of results */
}

/* map_get_item

Get list of item on a tile

Input:
 - ID of a map
 - Layer of the map
 - X coordinate (in tiles)
 - Y coordinate (in tiles)
Output: Array of item on that tile
*/
static int l_map_get_item( lua_State* L)
{
	const char * map;
	int layer;
	int x;
	int y;
	char ** res;
	char ** cur_res;
	int res_num = 0;

	map = luaL_checkstring(L, -4);
	layer = luaL_checkint(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);

	res = map_get_item(map,layer,x,y);
	if( res) {
		cur_res = res;
		while(*cur_res != NULL) {
			lua_pushstring(L, *cur_res);
			free(*cur_res);
			res_num++;
			cur_res++;
		}
		free(res);
	}

	return res_num;  /* number of results */
}

/* character_attribute_change

Add (or remove) a value to the given attribute

Input:
 - ID of a character
 - ID of an attribute
 - value to add (may be negative)
Output:
*/
static int l_character_attribute_change( lua_State* L)
{
	const char * id;
	const char * attribute;
	int value;
	int res;
	context_t * context;

	id = luaL_checkstring(L, -3);
	attribute = luaL_checkstring(L, -2);
	value = luaL_checkint(L, -1);

	lua_getglobal(L,LUAVM_CONTEXT);
	context = lua_touserdata(L, -1);
	lua_pop(L,1);

	res = attribute_change(context,CHARACTER_TABLE,id,attribute,value);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* character_attribute_get

Get the value of the given attribute

Input:
 - ID of a character
 - ID of an attribute
Output: Value of the given attribute or -1 if error
*/
static int l_character_attribute_get( lua_State* L)
{
	const char * id;
	const char * attribute;
	int res;

	id = luaL_checkstring(L, -2);
	attribute = luaL_checkstring(L, -1);
	res = attribute_get(CHARACTER_TABLE,id,attribute);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* character_attribute_set

Set an attribute to the given value

Input:
 - ID of a character
 - ID of an attribute
 - value
Output:
*/
static int l_character_attribute_set( lua_State* L)
{
	const char * id;
	const char * attribute;
	int value;
	int res;

	id = luaL_checkstring(L, -3);
	attribute = luaL_checkstring(L, -2);
	value = luaL_checkint(L, -1);
	res = attribute_set(CHARACTER_TABLE,id,attribute,value);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* map_attribute_change

Add (or remove) a value to the given attribute

Input:
 - ID of a map
 - ID of an attribute
 - value to add (may be negative)
Output:
*/
static int l_map_attribute_change( lua_State* L)
{
	const char * id;
	const char * attribute;
	int value;
	int res;
	context_t * context;

	id = luaL_checkstring(L, -3);
	attribute = luaL_checkstring(L, -2);
	value = luaL_checkint(L, -1);

	lua_getglobal(L,LUAVM_CONTEXT);
	context = lua_touserdata(L, -1);
	lua_pop(L,1);

	res = attribute_change(context,MAP_TABLE,id,attribute,value);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* map_attribute_get

Get the value of the given attribute

Input:
 - ID of a map
 - ID of an attribute
Output: Value of the given attribute
*/
static int l_map_attribute_get( lua_State* L)
{
	const char * id;
	const char * attribute;
	int res;

	id = luaL_checkstring(L, -2);
	attribute = luaL_checkstring(L, -1);
	res = attribute_get(MAP_TABLE,id,attribute);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* map_attribute_set

Set an attribute to the given value

Input:
 - ID of a map
 - ID of an attribute
 - value
Output:
*/
static int l_map_attribute_set( lua_State* L)
{
	const char * id;
	const char * attribute;
	int value;
	int res;

	id = luaL_checkstring(L, -3);
	attribute = luaL_checkstring(L, -2);
	value = luaL_checkint(L, -1);
	res = attribute_set(MAP_TABLE,id,attribute,value);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* equipment_slot_add_item

Add an item to an equipment slot

Input:
 - ID of a character
 - ID of an equipment slot
 - ID of an item
Output:
*/
static int l_equipment_slot_set_item( lua_State* L)
{
	const char * id;
	const char * slot;
	const char * item;
	int res;

	id = luaL_checkstring(L, -3);
	slot = luaL_checkstring(L, -2);
	item = luaL_checkstring(L, -1);
	res = equipment_set_item(id,slot,item);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* equipment_slot_get_item

Get the item's ID of an equipment slot

Input:
 - ID of a character
 - ID of an equipment slot
Output: ID of an item
*/
static int l_equipment_slot_get_item( lua_State* L)
{
	const char * id;
	const char * slot;
	char * item;

	id = luaL_checkstring(L, -2);
	slot = luaL_checkstring(L, -1);
	item = equipment_get_item(id,slot);
	if( item == NULL ) {
		return 0;  /* number of results */
	}
	lua_pushstring(L, item);
	free(item);
	return 1;  /* number of results */
}

/* popup_send

Send a popup screen to a context

Input:
 - ID of the target character
 - Array of string describing the pop-up
Output: -1 on error, 0 otherwise
*/
static int l_popup_send( lua_State* L)
{
	int num_arg;
	const char **arg = NULL;
	int i;

	num_arg = lua_gettop(L);

	arg = malloc(sizeof(char*)*num_arg+1);
	for(i=0; i<num_arg; i++) {
		arg[i] = luaL_checkstring(L, -num_arg+i);
	}
	arg[i]=NULL;

	network_send_popup(arg[0],&arg[1]);

	free(arg);
	lua_pushnumber(L, 0);
	return 1;  /* number of results */
}

/**************************************
**************************************/
static void action_chat(context_t * context, const char * text)
{
	char * new_text;

	new_text = strconcat(context->character_name,":",text,NULL);

	network_broadcast_text(context,new_text);

	free(new_text);
}

/**************************************
Execute a LUA script file
return -1 if the script do not return something
**************************************/
int action_execute_script(context_t * context, const char * script, char ** parameters)
{
	char * filename;
	int param_num = 0;
	int return_value;

	if(script == NULL) {
		return -1;
	}

	/* Special case for chat */
	if( strcmp(script,WOG_CHAT)==0) {
		action_chat(context,parameters[0]);
		return -1;
	}

	/* Load script */
	filename = strconcat(base_directory,"/",SCRIPT_TABLE,"/",script,NULL);

	if (luaL_loadfile(context->luaVM, filename) != 0 ) {
		/* If something went wrong, error message is at the top of */
		/* the stack */
		werr(LOGUSER,"Couldn't load LUA script %s: %s\n", filename, lua_tostring(context->luaVM, -1));
		free(filename);
		return -1;
	}

	/* Fake call to read global variable from the script file (i.e. the f function */
	lua_pcall(context->luaVM, 0, 0, 0);

	/* push f function on LUA VM stack */
	lua_getglobal(context->luaVM,"f");

	/* push parameters on lua VM stack (only strings parameters are supported) */
	if(parameters != NULL ) {
		while(parameters[param_num] != NULL ) {
			lua_pushstring(context->luaVM,parameters[param_num]);
			param_num++;
		}
	}

	/* Ask Lua to call the f function with the given parameters */
	if (lua_pcall(context->luaVM, param_num, 1, 0) != 0) {
		werr(LOGUSER,"Error running LUA script %s: %s\n", filename, lua_tostring(context->luaVM, -1));
		free(filename);
		return -1;
	}
	free(filename);

	/* retrieve result */
	if (!lua_isnumber(context->luaVM, -1)) {
		lua_pop(context->luaVM, 1);
		return -1;
	}
	return_value = lua_tonumber(context->luaVM, -1);
	lua_pop(context->luaVM, 1);
	return return_value;
}

/**************************************
Execute an action configuration file
return -1 if the script do not return something
**************************************/
int action_execute(context_t * context, const char * action, char ** parameters)
{
	char * script;
	char ** params;
	char ** all_params;
	int ret;

	if(!entry_read_string(ACTION_TABLE,action,&script,ACTION_KEY_SCRIPT,NULL)) {
		return -1;
	}
	entry_read_list(ACTION_TABLE,action,&params,ACTION_KEY_PARAM,NULL);

	all_params = add_array(params,parameters);

	ret = action_execute_script(context,script,all_params);

	deep_free(params);
	free(all_params);

	return ret;
}

/***************************************************
Call another script

Input:
 - script name
 - Array of parameters
Output: -1 on script error, if no error script return value
***************************************************/
static int l_call_script( lua_State* L)
{
	const char * script;
	int num_arg;
	char **arg = NULL;
	int i;
	int res;
	context_t * context;

	lua_getglobal(L,LUAVM_CONTEXT);
	context = lua_touserdata(L, -1);
	lua_pop(L,1);

	num_arg = lua_gettop(L);
	script = luaL_checkstring(L, -num_arg);
	if(num_arg > 1 ) {
		arg = malloc(sizeof(char*)*num_arg);
		for(i=0; i<num_arg-1; i++) {
			arg[i] = (char *)luaL_checkstring(L, -num_arg+1+i); /* FIXME wrong casting ? */
		}
		arg[i] = NULL; /* End of list */
	}

	res = action_execute_script(context, script, arg);

	if(arg) {
		free(arg);
	}
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/***************************************************
Call an action

Input:
 - action name
 - Array of parameters
Output: -1 on script error, if no error script return value
***************************************************/
static int l_call_action( lua_State* L)
{
	const char * action;
	int num_arg;
	char **arg = NULL;
	int i;
	int res;
	context_t * context;

	lua_getglobal(L,LUAVM_CONTEXT);
	context = lua_touserdata(L, -1);
	lua_pop(L,1);

	num_arg = lua_gettop(L);
	action = luaL_checkstring(L, -num_arg);
	if(num_arg > 1 ) {
		arg = malloc(sizeof(char*)*num_arg);
		for(i=0; i<num_arg-1; i++) {
			arg[i] = (char *)luaL_checkstring(L, -num_arg+1+i); /* FIXME wrong casting ? */
		}
		arg[i] = NULL; /* End of list */
	}

	res = action_execute(context, action, arg);

	if(arg) {
		free(arg);
	}
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/***************************************************
***************************************************/
void register_lua_functions(context_t * context)
{
	lua_State* L = context->luaVM;

	/* player func */
	lua_pushcfunction(L, l_player_get_id);
	lua_setglobal(L, "player_get_id");
	/* character func */
	lua_pushcfunction(L, l_character_create_from_template);
	lua_setglobal(L, "character_create_from_template");
	lua_pushcfunction(L, l_character_get_selected_map);
	lua_setglobal(L, "character_get_selected_map");
	lua_pushcfunction(L, l_character_get_selected_map_tile_x);
	lua_setglobal(L, "character_get_selected_map_tile_x");
	lua_pushcfunction(L, l_character_get_selected_map_tile_y);
	lua_setglobal(L, "character_get_selected_map_tile_y");
	lua_pushcfunction(L, l_character_set_selected_tile);
	lua_setglobal(L, "character_set_selected_tile");
	lua_pushcfunction(L, l_character_get_selected_inventory_id);
	lua_setglobal(L, "character_get_selected_inventory_id");
	lua_pushcfunction(L, l_character_set_selected_inventory_id);
	lua_setglobal(L, "character_set_selected_inventory_id");
	lua_pushcfunction(L, l_character_get_selected_equipment_slot);
	lua_setglobal(L, "character_get_selected_equipment_slot");
	lua_pushcfunction(L, l_character_set_selected_equipment_slot);
	lua_setglobal(L, "character_set_selected_equipment_slot");
	lua_pushcfunction(L, l_character_get_selected_character_id);
	lua_setglobal(L, "character_get_selected_character_id");
	lua_pushcfunction(L, l_character_set_selected_character_id);
	lua_setglobal(L, "character_set_selected_character_id");
	lua_pushcfunction(L, l_character_get_map);
	lua_setglobal(L, "character_get_map");
	lua_pushcfunction(L, l_character_get_map_w);
	lua_setglobal(L, "character_get_map_w");
	lua_pushcfunction(L, l_character_get_map_h);
	lua_setglobal(L, "character_get_map_h");
	lua_pushcfunction(L, l_character_get_x);
	lua_setglobal(L, "character_get_x");
	lua_pushcfunction(L, l_character_get_y);
	lua_setglobal(L, "character_get_y");
	lua_pushcfunction(L, l_character_get_name);
	lua_setglobal(L, "character_get_name");
	lua_pushcfunction(L, l_character_get_type);
	lua_setglobal(L, "character_get_type");
	lua_pushcfunction(L, l_character_set_pos);
	lua_setglobal(L, "character_set_pos");
	lua_pushcfunction(L, l_character_set_npc);
	lua_setglobal(L, "character_set_npc");
	lua_pushcfunction(L, l_character_get_npc);
	lua_setglobal(L, "character_get_npc");
	lua_pushcfunction(L, l_character_get_portrait);
	lua_setglobal(L, "character_get_portrait");
	lua_pushcfunction(L, l_character_set_portrait);
	lua_setglobal(L, "character_set_portrait");
	lua_pushcfunction(L, l_character_out_of_game);
	lua_setglobal(L, "character_out_of_game");
	lua_pushcfunction(L, l_character_disconnect);
	lua_setglobal(L, "character_disconnect");
	lua_pushcfunction(L, l_character_delete);
	lua_setglobal(L, "character_delete");
	/* map func */
	lua_pushcfunction(L, l_map_new);
	lua_setglobal(L, "map_new");
	lua_pushcfunction(L, l_map_set_tile);
	lua_setglobal(L, "map_set_tile");
	lua_pushcfunction(L, l_map_set_tile_type);
	lua_setglobal(L, "map_set_tile_type");
	lua_pushcfunction(L, l_map_add_item);
	lua_setglobal(L, "map_add_item");
	lua_pushcfunction(L, l_map_delete_item);
	lua_setglobal(L, "map_delete_item");
	lua_pushcfunction(L, l_map_get_tile);
	lua_setglobal(L, "map_get_tile");
	lua_pushcfunction(L, l_map_get_tile_type);
	lua_setglobal(L, "map_get_tile_type");
	lua_pushcfunction(L, l_map_get_character);
	lua_setglobal(L, "map_get_character");
	lua_pushcfunction(L, l_map_get_item);
	lua_setglobal(L, "map_get_item");
	lua_pushcfunction(L, l_map_add_event);
	lua_setglobal(L, "map_add_event");
	lua_pushcfunction(L, l_map_add_event_param);
	lua_setglobal(L, "map_add_event_param");
	lua_pushcfunction(L, l_map_delete_event);
	lua_setglobal(L, "map_delete_event");
	/* inventory func */
	lua_pushcfunction(L, l_inventory_delete);
	lua_setglobal(L, "inventory_delete");
	lua_pushcfunction(L, l_inventory_add);
	lua_setglobal(L, "inventory_add");
	lua_pushcfunction(L, l_inventory_get_by_name);
	lua_setglobal(L, "inventory_get_by_name");
	/* item func */
	lua_pushcfunction(L, l_item_create_empty);
	lua_setglobal(L, "item_create_empty");
	lua_pushcfunction(L, l_item_create_from_template);
	lua_setglobal(L, "item_create_from_template");
	lua_pushcfunction(L, l_resource_new);
	lua_setglobal(L, "resource_new");
	lua_pushcfunction(L, l_resource_get_quantity);
	lua_setglobal(L, "resource_get_quantity");
	lua_pushcfunction(L, l_resource_set_quantity);
	lua_setglobal(L, "resource_set_quantity");
	lua_pushcfunction(L, l_item_destroy);
	lua_setglobal(L, "item_destroy");
	/* character attribute func */
	lua_pushcfunction(L, l_character_attribute_change);
	lua_setglobal(L, "character_attribute_change");
	lua_pushcfunction(L, l_character_attribute_get);
	lua_setglobal(L, "character_attribute_get");
	lua_pushcfunction(L, l_character_attribute_set);
	lua_setglobal(L, "character_attribute_set");
	/* map attribute func */
	lua_pushcfunction(L, l_map_attribute_change);
	lua_setglobal(L, "map_attribute_change");
	lua_pushcfunction(L, l_map_attribute_get);
	lua_setglobal(L, "map_attribute_get");
	lua_pushcfunction(L, l_map_attribute_set);
	lua_setglobal(L, "map_attribute_set");
	/* equipment func */
	lua_pushcfunction(L, l_equipment_slot_set_item);
	lua_setglobal(L, "equipment_slot_set_item");
	lua_pushcfunction(L, l_equipment_slot_get_item);
	lua_setglobal(L, "equipment_slot_get_item");
	/* misc func */
	lua_pushcfunction(L, l_popup_send);
	lua_setglobal(L, "popup_send");
	lua_pushcfunction(L, l_print_text_id);
	lua_setglobal(L, "print_text_id");
	lua_pushcfunction(L, l_print_text_map);
	lua_setglobal(L, "print_text_map");
	lua_pushcfunction(L, l_print_text_server);
	lua_setglobal(L, "print_text_server");
	lua_pushcfunction(L, l_print_text_debug);
	lua_setglobal(L, "print_text_debug");
	lua_pushcfunction(L, l_call_script);
	lua_setglobal(L, "call_script");
	lua_pushcfunction(L, l_call_action);
	lua_setglobal(L, "call_action");

	/* push the context on lua VM stack */
	lua_pushlightuserdata(L,context);
	lua_setglobal (L, LUAVM_CONTEXT);
}

