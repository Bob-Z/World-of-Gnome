/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013-2014 carabobz@gmail.com

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
 - x ...
 - y ...
Output:
	id of the new character or nil if cannot be created or placed
*/
static int l_character_create_from_template( lua_State* L)
{
	const char * template;
	const char * map;
	int x;
	int y;

	char * res;
	context_t * ctx;

	lua_getglobal(L,LUAVM_CONTEXT);
	ctx = lua_touserdata(L, -1);
	lua_pop(L,1);

	template = luaL_checkstring(L, -4);
	map = luaL_checkstring(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	res = character_create_from_template(ctx,template,map,x,y);
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

/* character_get_map_x
Input:
 - ID of a map
Output: Width of the map
*/
static int l_character_get_map_x( lua_State* L)
{
	context_t * target;
	const char * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		werr(LOGDEV,"Cannot find context with ID %s",id);
		return 0;  /* number of results */
	}
	lua_pushnumber(L, target->map_x);
	return 1;  /* number of results */
}

/* character_get_map_y
Input:
 - ID of a map
Output: Height of the map
*/
static int l_character_get_map_y( lua_State* L)
{
	context_t * target;
	const char * id;

	id = luaL_checkstring(L, -1);
	target = context_find(id);
	if( target == NULL ) {
		werr(LOGDEV,"Cannot find context with ID %s",id);
		return 0;  /* number of results */
	}
	lua_pushnumber(L, target->map_y);
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

/* character_disconnect

Disconnect a context

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

Delete the data file of a character

Input:
 - ID of a character
Output:
*/
static int l_character_delete( lua_State* L)
{
	const char * id;
	int res;

	id = luaL_checkstring(L, -1);
	res = entry_destroy(id);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* character_set_pos

Set a character's position.
May fail with regard to allowed tiles

Input:
 - ID of a character
 - ID of the map to set
 - X cooridnate in the map
 - Y cooridnate in the map
Output:
*/
static int l_character_set_pos( lua_State* L)
{
	const char * id;
	const char * map;
	int x;
	int y;
	int res;
	context_t * ctx;

	id = luaL_checkstring(L, -4);
	map = luaL_checkstring(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);

	ctx = context_find(id);
	res = character_set_pos(ctx,map,x,y);
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

/* map_new

Create a map

Input:
 - Width of map
 - Height of map
 - Width of tile (in pixels)
 - Height of tile (in pixels)
 - ID of default tile (new map is filled with)
Output: New map ID
*/
static int l_map_new( lua_State* L)
{
	char * map_name;
	int x;
	int y;
	int tile_x;
	int tile_y;
	const char * default_tile;

	x = luaL_checkint(L, -5);
	y = luaL_checkint(L, -4);
	tile_x = luaL_checkint(L, -3);
	tile_y = luaL_checkint(L, -2);
	default_tile = luaL_checkstring(L, -1);
	map_name = map_new(x,y,tile_x,tile_y,(char *)default_tile);
	lua_pushstring(L, map_name);
	free(map_name);
	return 1;  /* number of results */
}

/* map_set_tile

Set a tile in a map

Input:
 - ID of a map
 - ID of a tile
 - X coordinate of the tile to set
 - Y coordinate of the tile to set
Output:
*/
static int l_map_set_tile( lua_State* L)
{
	const char * map;
	const char * tile;
	int x;
	int y;
	int res;

	map = luaL_checkstring(L, -4);
	tile = luaL_checkstring(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	res = map_set_tile(map,tile,x,y);
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
 - ID of an item
 - X coordinate in the map
 - Y coordinate in the map
Output:
*/
static int l_map_add_item( lua_State* L)
{
	const char * map;
	const char * item;
	int x;
	int y;
	int res;

	map = luaL_checkstring(L, -4);
	item = luaL_checkstring(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	res = map_add_item(map,item,x,y);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* map_delete_item

Remove an item from a map (only the first item found on the tile is deleted)

Input:
 - ID of a map
 - X coordinate in the map
 - Y coordinate in the map
Output:
*/
static int l_map_delete_item( lua_State* L)
{
	const char * map;
	int x;
	int y;
	char * res;

	map = luaL_checkstring(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	res = map_delete_item(map,x,y);
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
 - Event's script
 - X coordinate in the map
 - Y coordinate in the map
Output: Event ID
*/
static int l_map_add_event( lua_State* L)
{
	const char * map;
	const char * script;
	int x;
	int y;
	char *res;

	map = luaL_checkstring(L, -4);
	script = luaL_checkstring(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	res = map_add_event(map,script,x,y);
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
 - ID of an event
 - parameter to add
Output:
*/
static int l_map_add_event_param( lua_State* L)
{
	const char * map;
	const char * event_id;
	const char * param;
	int res;

	map = luaL_checkstring(L, -3);
	event_id = luaL_checkstring(L, -2);
	param = luaL_checkstring(L, -1);
	res = map_add_event_param(map,event_id,param);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* map_delete_event

Add a parameter to the given event

Input:
 - ID of a map
 - event's script
 - X coordinate (in tiles)
 - Y coordinate (in tiles)
Output:
*/
static int l_map_delete_event( lua_State* L)
{
	const char * map;
	const char * script;
	int x;
	int y;
	int res;

	map = luaL_checkstring(L, -4);
	script = luaL_checkstring(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	res = map_delete_event(map,script,x,y);
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

/* item_get_quantity

Get the quantity of an item

Input:
 - ID of an item
Output: Quantity of that item
*/
static int l_item_get_quantity( lua_State* L)
{
	const char * item;
	int res;

	item = luaL_checkstring(L, -1);
	res = item_get_quantity(item);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* item_set_quantity

Set the quantity of an item

Input:
 - ID of an item
 - quantity to set
Output: -1 on error
*/
static int l_item_set_quantity( lua_State* L)
{
	const char * item;
	int quantity;
	int res;

	item = luaL_checkstring(L, -2);
	quantity = luaL_checkint(L, -1);
	res = item_set_quantity(item,quantity);
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
 - X coordinate (in tiles)
 - Y coordinate (in tiles)
Output: ID of the tile
*/
static int l_map_get_tile( lua_State* L)
{
	const char * map;
	int x;
	int y;
	char * res;

	map = luaL_checkstring(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	res = map_get_tile(map,x,y);
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
 - X coordinate (in tiles)
 - Y coordinate (in tiles)
Output: type of the tile
*/
static int l_map_get_tile_type( lua_State* L)
{
	const char * map;
	int x;
	int y;
	char * res;

	map = luaL_checkstring(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	res = map_get_tile_type(map,x,y);
	lua_pushstring(L, res);
	free(res);
	return 1;  /* number of results */
}

/* map_get_character

Get list of characters on a tile

Input:
 - ID of a map
 - X coordinate (in tiles)
 - Y coordinate (in tiles)
Output: Array of characters on that tile
*/
static int l_map_get_character( lua_State* L)
{
	const char * map;
	int x;
	int y;
	char ** res;
	char ** cur_res;
	int res_num = 0;

	map = luaL_checkstring(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);

	res = map_get_character(map,x,y);
	if( res) {
		cur_res = res;
		while(*cur_res != NULL) {
//			wlog(LOGDEBUG,"Pushing %s",*cur_res);
			lua_pushstring(L, *cur_res);
			free(*cur_res);
			res_num++;
			cur_res++;
		}
		free(res);
	}

//wlog(LOGDEBUG,"returning %d",res_num);
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

	res = attribute_change(context,id,attribute,value);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* character_attribute_get

Get the value of the given attribute

Input:
 - ID of a character
 - ID of an attribute
Output: Value of the given attribute
*/
static int l_character_attribute_get( lua_State* L)
{
	const char * id;
	const char * attribute;
	int res;

	id = luaL_checkstring(L, -2);
	attribute = luaL_checkstring(L, -1);
	res = attribute_get(id,attribute);
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
	res = attribute_set(id,attribute,value);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* equipment_slot_delete_item

Delete an item from an equipment slot

Input:
 - ID of a character
 - ID of an equipment slot
Output:
*/
static int l_equipment_slot_delete_item( lua_State* L)
{
	const char * id;
	const char * slot;
	int res;

	id = luaL_checkstring(L, -2);
	slot = luaL_checkstring(L, -1);
	res = equipment_delete(id,slot);
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
static int l_equipment_slot_add_item( lua_State* L)
{
	const char * id;
	const char * slot;
	const char * item;
	int res;

	id = luaL_checkstring(L, -3);
	slot = luaL_checkstring(L, -2);
	item = luaL_checkstring(L, -1);
	res = equipment_add(id,slot,item);
	lua_pushnumber(L, res);
	return 1;  /* number of results */
}

/* equipment_slot_get_item_id

Get the item's ID of an equipment slot

Input:
 - ID of a character
 - ID of an equipment slot
Output: ID of an item
*/
static int l_equipment_slot_get_item_id( lua_State* L)
{
	const char * id;
	const char * slot;
	char * item;

	id = luaL_checkstring(L, -2);
	slot = luaL_checkstring(L, -1);
	item = equipment_get_item_id(id,slot);
	if( item == NULL ) {
		return 0;  /* number of results */
	}
	lua_pushstring(L, item);
	free(item);
	return 1;  /* number of results */
}

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
	lua_pushcfunction(L, l_character_get_selected_inventory_id);
	lua_setglobal(L, "character_get_selected_inventory_id");
	lua_pushcfunction(L, l_character_get_selected_equipment_slot);
	lua_setglobal(L, "character_get_selected_equipment_slot");
	lua_pushcfunction(L, l_character_get_selected_character_id);
	lua_setglobal(L, "character_get_selected_character_id");
	lua_pushcfunction(L, l_character_get_map);
	lua_setglobal(L, "character_get_map");
	lua_pushcfunction(L, l_character_get_map_x);
	lua_setglobal(L, "character_get_map_x");
	lua_pushcfunction(L, l_character_get_map_y);
	lua_setglobal(L, "character_get_map_y");
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
	lua_pushcfunction(L, l_character_disconnect);
	lua_setglobal(L, "character_disconnect");
	lua_pushcfunction(L, l_character_delete);
	lua_setglobal(L, "character_delete");
	/* map func */
	lua_pushcfunction(L, l_map_new);
	lua_setglobal(L, "map_new");
	lua_pushcfunction(L, l_map_set_tile);
	lua_setglobal(L, "map_set_tile");
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
	lua_pushcfunction(L, l_item_get_quantity);
	lua_setglobal(L, "item_get_quantity");
	lua_pushcfunction(L, l_item_set_quantity);
	lua_setglobal(L, "item_set_quantity");
	lua_pushcfunction(L, l_item_destroy);
	lua_setglobal(L, "item_destroy");
	/* character attribute func */
	lua_pushcfunction(L, l_character_attribute_change);
	lua_setglobal(L, "character_attribute_change");
	lua_pushcfunction(L, l_character_attribute_get);
	lua_setglobal(L, "character_attribute_get");
	lua_pushcfunction(L, l_character_attribute_set);
	lua_setglobal(L, "character_attribute_set");
	/* equipment func */
	lua_pushcfunction(L, l_equipment_slot_delete_item);
	lua_setglobal(L, "equipment_slot_delete_item");
	lua_pushcfunction(L, l_equipment_slot_add_item);
	lua_setglobal(L, "equipment_slot_get_add_item");
	lua_pushcfunction(L, l_equipment_slot_get_item_id);
	lua_setglobal(L, "equipment_slot_get_item_id");
	/* misc func */
	lua_pushcfunction(L, l_print_text_id);
	lua_setglobal(L, "print_text_id");
	lua_pushcfunction(L, l_print_text_map);
	lua_setglobal(L, "print_text_map");
	lua_pushcfunction(L, l_print_text_server);
	lua_setglobal(L, "print_text_server");
	lua_pushcfunction(L, l_print_text_debug);
	lua_setglobal(L, "print_text_debug");

	/* push the context on lua VM stack */
	lua_pushlightuserdata(L,context);
	lua_setglobal (L, LUAVM_CONTEXT);
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
	filename = strconcat(getenv("HOME"),"/",base_directory,"/",SCRIPT_TABLE,"/",script,NULL);

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
		werr(LOGUSER,"Failed to run LUA script %s: %s\n", filename, lua_tostring(context->luaVM, -1));
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
