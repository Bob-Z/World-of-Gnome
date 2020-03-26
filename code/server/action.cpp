/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2020 carabobz@gmail.com

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

#include "attribute.h"
#include "character.h"
#include "client_server.h"
#include "context_server.h"
#include "entry.h"
#include "equipment.h"
#include "file.h"
#include "inventory.h"
#include "item.h"
#include "log.h"
#include "lua_script.h"
#include "map_server.h"
#include "network_server.h"
#include "network.h"
#include "protocol.h"
#include "syntax.h"
#include "util.h"
#include <cstring>
#include <stdlib.h>
#include <string>
#include <vector>

#ifdef __cplusplus
extern "C"
{
#endif

#include <lualib.h>
#include <lauxlib.h>

#ifdef __cplusplus
}
#endif

#define LUAVM_CONTEXT "wog_context"

// LUA script functions

/* player_get_id
 Input:
 Output: ID of the current context
 */
static int l_player_get_id(lua_State* L)
{
	Context * context;

	lua_getglobal(L, LUAVM_CONTEXT);
	context = (Context*) lua_touserdata(L, -1);
	lua_pop(L, 1);
	lua_pushstring(L, context->getId().c_str());
	return 1;  // number of results
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
static int l_character_create_from_template(lua_State* L)
{
	const char * mytemplate = nullptr;
	const char * map = nullptr;
	int layer = -1;
	int x = -1;
	int y = -1;

	Context * ctx;

	lua_getglobal(L, LUAVM_CONTEXT);
	ctx = (Context*) lua_touserdata(L, -1);
	lua_pop(L, 1);

	mytemplate = luaL_checkstring(L, -5);
	map = luaL_checkstring(L, -4);
	layer = luaL_checkint(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	const std::pair<bool, std::string> res = character_create_from_template(ctx, mytemplate, map, layer, x, y);
	lua_pushstring(L, res.second.c_str());

	return 1;  // number of results
}

/* character_get_selected_map_tile_x
 Input:
 - ID of a character
 Output: X coordinate if selected tile
 */
static int l_character_get_selected_map_tile_x(lua_State* L)
{
	Context * target;
	const char * id;

	id = luaL_checkstring(L, -1);
	target = context_find(std::string(id));
	if (target == nullptr)
	{
		werr(LOGDESIGNER, "Cannot find context with ID %s", id);
		return 0;  // number of results
	}
	lua_pushnumber(L, target->getSelectionMapTx());
	return 1;  // number of results
}

/* character_get_selected_map_tile_y
 Input:
 - ID of a character
 Output: Y coordinate if selected tile
 */
static int l_character_get_selected_map_tile_y(lua_State* L)
{
	Context * target;
	const char * id;

	id = luaL_checkstring(L, -1);
	target = context_find(std::string(id));
	if (target == nullptr)
	{
		werr(LOGDESIGNER, "Cannot find context with ID %s", id);
		return 0;  // number of results
	}
	lua_pushnumber(L, target->getSelectionMapTy());
	return 1;  // number of results
}

/* character_get_selected_map
 Input:
 - ID of a character
 Output: ID of the selected map
 */
static int l_character_get_selected_map(lua_State* L)
{
	Context * target;
	const char * id;

	id = luaL_checkstring(L, -1);
	target = context_find(std::string(id));
	if (target == nullptr)
	{
		werr(LOGDESIGNER, "Cannot find context with ID %s", id);
		return 0;  // number of results
	}
	lua_pushstring(L, target->getSelectionMap().c_str());
	return 1;  // number of results
}

/* character_set_selected_tile
 Input:
 - ID of a character
 - ID of a map
 - X coord of the selected tile in this map
 - Y coord of the selected tile in this map
 Output:
 */
static int l_character_set_selected_tile(lua_State* L)
{
	Context * target;
	const char * id;
	const char * selected_map;
	int tx;
	int ty;

	id = luaL_checkstring(L, -4);
	selected_map = luaL_checkstring(L, -3);
	tx = luaL_checkint(L, -2);
	ty = luaL_checkint(L, -1);

	target = context_find(std::string(id));
	if (target == nullptr)
	{
		werr(LOGDESIGNER, "Cannot find context with ID %s", id);
		return 0;  // number of results
	}

	target->setSelectionTile(std::string(selected_map), tx, ty);

	network_send_context_to_context(target, target);

	return 0;  // number of results
}

/* character_get_selected_inventory_id
 Input:
 - ID of a character
 Output: ID of selected item in inventory
 */
static int l_character_get_selected_inventory_id(lua_State* L)
{
	Context * target;
	const char * id;

	id = luaL_checkstring(L, -1);
	target = context_find(std::string(id));
	if (target == nullptr)
	{
		werr(LOGDESIGNER, "Cannot find context with ID %s", id);
		return 0;  // number of results
	}
	lua_pushstring(L, target->getSelectionInventory().c_str());
	return 1;  // number of results
}

/* character_set_selected_inventory_id
 Input:
 - ID of a character
 - ID of an item
 Output:
 */
static int l_character_set_selected_inventory_id(lua_State* L)
{
	Context * target;
	const char * id;
	const char * selected_item;

	id = luaL_checkstring(L, -2);
	selected_item = luaL_checkstring(L, -1);

	target = context_find(std::string(id));
	if (target == nullptr)
	{
		werr(LOGDESIGNER, "Cannot find context with ID %s", id);
		return 0;  // number of results
	}

	target->setSelectionInventory(selected_item);

	network_send_context_to_context(target, target);

	return 0;  // number of results
}

/* character_get_selected_equipment_slot
 Input:
 - ID of a character
 Output: ID of selected item in equipment
 */
static int l_character_get_selected_equipment_slot(lua_State* L)
{
	Context * target;
	const char * id;

	id = luaL_checkstring(L, -1);
	target = context_find(std::string(id));
	if (target == nullptr)
	{
		werr(LOGDESIGNER, "Cannot find context with ID %s", id);
		return 0;  // number of results
	}
	lua_pushstring(L, target->getSelectionEquipment().c_str());
	return 1;  // number of results
}

/* character_set_selected_equipment_slot
 Input:
 - ID of a character
 - ID of an equipment slot
 Output:
 */
static int l_character_set_selected_equipment_slot(lua_State* L)
{
	Context * target;
	const char * id;
	const char * selected_equipment;

	id = luaL_checkstring(L, -2);
	selected_equipment = luaL_checkstring(L, -1);

	target = context_find(std::string(id));
	if (target == nullptr)
	{
		werr(LOGDESIGNER, "Cannot find context with ID %s", id);
		return 0;  // number of results
	}

	target->setSelectionEquipment(selected_equipment);

	network_send_context_to_context(target, target);

	return 0;  // number of results
}

/* character_get_selected_character_id
 Input:
 - ID of a character
 Output: ID of the selected character
 */
static int l_character_get_selected_character_id(lua_State* L)
{
	Context * target;
	const char * id;

	id = luaL_checkstring(L, -1);
	target = context_find(std::string(id));
	if (target == nullptr)
	{
		werr(LOGDESIGNER, "Cannot find context with ID %s", id);
		return 0;  // number of results
	}
	lua_pushstring(L, target->getSelectionContextId().c_str());
	return 1;  // number of results
}

/* character_set_selected_character_id
 Input:
 - ID of a character
 - ID of selected character
 Output:
 */
static int l_character_set_selected_character_id(lua_State* L)
{
	Context * target;
	const char * id;
	const char * selected_id;

	id = luaL_checkstring(L, -2);
	selected_id = luaL_checkstring(L, -1);

	target = context_find(std::string(id));
	if (target == nullptr)
	{
		werr(LOGDESIGNER, "Cannot find context with ID %s", id);
		return 0;  // number of results
	}

	target->setSelectionContextId(selected_id);

	network_send_context_to_context(target, target);

	return 0;  // number of results
}

/* character_get_map
 Input:
 - ID of a character
 Output: ID of the map where the character is
 */
static int l_character_get_map(lua_State* L)
{
	Context * target;
	const char * id;

	id = luaL_checkstring(L, -1);
	target = context_find(std::string(id));
	if (target == nullptr)
	{
		werr(LOGDESIGNER, "Cannot find context with ID %s", id);
		return 0;  // number of results
	}
	lua_pushstring(L, target->getMap().c_str());
	return 1;  // number of results
}

/* character_get_map_w
 Input:
 - map name
 Output: Width of the map
 -1 if map does not exists
 */
static int l_character_get_map_w(lua_State* L)
{
	const char * map;
	int map_w = -1;

	map = luaL_checkstring(L, -1);
	entry_read_int(MAP_TABLE, map, &map_w, MAP_KEY_WIDTH, nullptr);

	lua_pushnumber(L, map_w);
	return 1;  // number of results
}

/* character_get_map_h
 Input:
 - map name
 Output: Height of the map
 -1 if map does not exists
 */
static int l_character_get_map_h(lua_State* L)
{
	const char * map;
	int map_h = -1;

	map = luaL_checkstring(L, -1);
	entry_read_int(MAP_TABLE, map, &map_h, MAP_KEY_HEIGHT, nullptr);

	lua_pushnumber(L, map_h);
	return 1;  // number of results
}

/* character_get_x
 Input:
 - ID of a character
 Output: X coordinate of the character
 */
static int l_character_get_x(lua_State* L)
{
	Context * target;
	const char * id;

	id = luaL_checkstring(L, -1);
	target = context_find(std::string(id));
	if (target == nullptr)
	{
		werr(LOGDESIGNER, "Cannot find context with ID %s", id);
		return 0;  // number of results
	}
	lua_pushnumber(L, target->getTileX());
	return 1;  // number of results
}

/* character_get_y
 Input:
 - ID of a character
 Output: Y coordinate of the character
 */
static int l_character_get_y(lua_State* L)
{
	Context * target;
	const char * id;

	id = luaL_checkstring(L, -1);
	target = context_find(std::string(id));
	if (target == nullptr)
	{
		werr(LOGDESIGNER, "Cannot find context with ID %s", id);
		return 0;  // number of results
	}
	lua_pushnumber(L, target->getTileY());
	return 1;  // number of results
}

/* character_get_name
 Input:
 - ID of a character
 Output: Name of the character
 */
static int l_character_get_name(lua_State* L)
{
	Context * target;
	const char * id;

	id = luaL_checkstring(L, -1);
	target = context_find(std::string(id));
	if (target == nullptr)
	{
		werr(LOGDESIGNER, "Cannot find context with ID %s", id);
		return 0;  // number of results
	}
	lua_pushstring(L, target->getCharacterName().c_str());
	return 1;  // number of results
}

/* character_get_type
 Input:
 - ID of a character
 Output: Type of the character
 */
static int l_character_get_type(lua_State* L)
{
	Context * target;
	const char * id;

	id = luaL_checkstring(L, -1);
	target = context_find(std::string(id));
	if (target == nullptr)
	{
		werr(LOGDESIGNER, "Cannot find context with ID %s", id);
		return 0;  // number of results
	}
	lua_pushstring(L, target->getType().c_str());
	return 1;  // number of results
}

/* character_out_of_game

 Kick a context out of the game.
 This does not disconnect it.

 Input:
 - ID of a character
 Output:
 */
static int l_character_out_of_game(lua_State* L)
{
	const char * id;
	int res;

	id = luaL_checkstring(L, -1);
	res = character_out_of_game(id);
	lua_pushnumber(L, res);
	return 1;  // number of results
}
/* character_disconnect

 Disconnect a context
 This kills a NPC AI

 Input:
 - ID of a character
 Output:
 */
static int l_character_disconnect(lua_State* L)
{
	const char * id;
	int res;

	id = luaL_checkstring(L, -1);
	res = character_disconnect(id);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* character_delete

 Delete the memory and file used by a character

 Input:
 - ID of a character
 Output:
 0 if success
 */
static int l_character_delete(lua_State* L)
{
	const char * id;
	int res;

	id = luaL_checkstring(L, -1);
	res = entry_destroy(CHARACTER_TABLE, id);
	if (res == 0)
	{
		res = file_delete(CHARACTER_TABLE, std::string(id));
	}
	lua_pushnumber(L, res);
	return 1;  // number of results
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
static int l_character_set_pos(lua_State* L)
{
	const char * id;
	const char * map;
	int x;
	int y;
	int res;
	Context * ctx;

	id = luaL_checkstring(L, -4);
	map = luaL_checkstring(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);

	ctx = context_find(id);
	res = character_set_pos(ctx, std::string(map), x, y);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* character_set_npc

 Set a character as a non-player character and starts it's AI script.

 Input:
 - ID of a character
 - Value of NPC (0 for player's character, 1 for NPC)
 Output:
 */
static int l_character_set_npc(lua_State* L)
{
	const char * id;
	int npc;
	int res;

	id = luaL_checkstring(L, -2);
	npc = luaL_checkint(L, -1);

	res = character_set_npc(id, npc);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* character_get_npc

 Get a characters npc attribute.

 Input:
 - ID of a character
 Output:
 - 0 if not NPC
 - 1 if NPC
 */
static int l_character_get_npc(lua_State* L)
{
	const char * id;
	int res;

	id = luaL_checkstring(L, -1);

	res = character_get_npc(std::string(id));
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* character_get_portrait

 Get portrait image file name

 Input:
 - ID of a character
 Output: portrait file name
 */
static int l_character_get_portrait(lua_State* L)
{
	const char * id;
	char * res;

	id = luaL_checkstring(L, -1);

	res = character_get_portrait(id);
	lua_pushstring(L, res);
	if (res)
	{
		free(res);
	}
	return 1;  // number of results
}

/* character_set_portrait

 Set a character's portrait.

 Input:
 - ID of a character
 - filename of the portrait
 Output:
 */
static int l_character_set_portrait(lua_State* L)
{
	const char * id;
	const char * portrait;
	int res;

	id = luaL_checkstring(L, -2);
	portrait = luaL_checkstring(L, -1);

	res = character_set_portrait(id, portrait);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* character_set_ai_script

 Set a character's AI script

 Input:
 - ID of a character
 - name of the AI script
 Output:
 */
static int l_character_set_ai_script(lua_State* L)
{
	const char * id;
	const char * script_name;
	int res;

	id = luaL_checkstring(L, -2);
	script_name = luaL_checkstring(L, -1);

	res = character_set_ai_script(id, script_name);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* character_set sprite

 Set a character's sprite

 Input:
 - ID of a character
 - index in the sprite list
 - name of the sprite file
 Output:
 */
static int l_character_set_sprite(lua_State* L)
{
	const char * id;
	int index;
	const char * sprite_name;
	int res;

	id = luaL_checkstring(L, -3);
	index = luaL_checkint(L, -2);
	sprite_name = luaL_checkstring(L, -1);

	res = character_set_sprite(id, index, sprite_name);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* character_set_sprite_dir

 Set a character's sprite for one direction

 Input:
 - ID of a character
 - direction to add (N,S,W,E)
 - index in the sprite list
 - name of the sprite file
 Output:
 */
static int l_character_set_sprite_dir(lua_State* L)
{
	const char * id;
	const char * dir;
	int index;
	const char * sprite_name;
	int res;

	id = luaL_checkstring(L, -4);
	dir = luaL_checkstring(L, -3);
	index = luaL_checkint(L, -2);
	sprite_name = luaL_checkstring(L, -1);

	res = character_set_sprite_dir(id, dir, index, sprite_name);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* character_set_sprite_move

 Set a character's moving sprite for one direction

 Input:
 - ID of a character
 - direction to add (N,S,W,E)
 - index in the sprite list
 - name of the sprite file
 Output:
 */
static int l_character_set_sprite_move(lua_State* L)
{
	const char * id;
	const char * dir;
	int index;
	const char * sprite_name;
	int res;

	id = luaL_checkstring(L, -4);
	dir = luaL_checkstring(L, -3);
	index = luaL_checkint(L, -2);
	sprite_name = luaL_checkstring(L, -1);

	res = character_set_sprite_move(id, dir, index, sprite_name);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* character_broadcast

 Send character to all context on the same map.
 Call this function after character_set_*_sprite functions

 Input:
 - ID of a character
 Output:
 */
static int l_character_broadcast(lua_State* L)
{
	const char * character;
	int res = 0;

	character = luaL_checkstring(L, -1);
	character_broadcast(character);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* character_effect

 Send character effect to all context on the same map.

 Input:
 - ID of a character
 - Array of string describing the effect
 Output: -1 on error, 0 otherwise
 */
static int l_character_effect(lua_State* L)
{
	const int l_NumArg = lua_gettop(L);

	std::string target_id = luaL_checkstring(L, -l_NumArg);

	std::vector<std::string> l_Param;
	for (int l_Idx = 1; l_Idx < l_NumArg; l_Idx++) // 1 because 0 is the target
	{
		l_Param.push_back(luaL_checkstring(L, -l_NumArg + l_Idx));
	}

	network_broadcast_effect(EffectManager::EffectType::CONTEXT, target_id, l_Param);

	lua_pushnumber(L, 0);
	return 1;  // number of results
}

/* character_wake_up

 Wake-up a NPC

 Input:
 - ID of a character
 Output:
 */
static int l_character_wake_up(lua_State* L)
{
	const char * id;
	int res;

	id = luaL_checkstring(L, -1);

	res = character_wake_up(id);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* map_new

 Create a map

 Input:
 - file name (if empty an available name is automatically found)
 - Width of map
 - Height of map
 - Width of tile (in pixels)
 - Height of tile (in pixels)
 Output: New map ID
 */
static int l_map_new(lua_State* L)
{
	const char * name = nullptr;
	int x = -1;
	int y = -1;
	int tile_x = -1;
	int tile_y = -1;

	name = luaL_checkstring(L, -5);
	x = luaL_checkint(L, -4);
	y = luaL_checkint(L, -3);
	tile_x = luaL_checkint(L, -2);
	tile_y = luaL_checkint(L, -1);
	const std::pair<bool, std::string> map_name = map_new(name, x, y, tile_x, tile_y);
	lua_pushstring(L, map_name.second.c_str());

	return 1;  // number of results
}

/* map_add_layer

 Add a layer to a map

 Input:
 - map name
 - layer to be created
 - Width of map
 - Height of map
 - Width of tile (in pixels)
 - Height of tile (in pixels)
 - default tile (new map is filled with)
 - default type (new map is filled with)
 Output: New map ID
 */
static int l_map_add_layer(lua_State* L)
{
	const char * map_name;
	int layer;
	int x;
	int y;
	int tile_x;
	int tile_y;
	const char * default_tile;
	const char * default_type;
	int res;

	map_name = luaL_checkstring(L, -8);
	layer = luaL_checkint(L, -7);
	x = luaL_checkint(L, -6);
	y = luaL_checkint(L, -5);
	tile_x = luaL_checkint(L, -4);
	tile_y = luaL_checkint(L, -3);
	default_tile = luaL_checkstring(L, -2);
	default_type = luaL_checkstring(L, -1);
	res = map_add_layer(map_name, layer, x, y, tile_x, tile_y, default_tile, default_type);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* map_delete_layer

 delete a layer on a map

 Input:
 - map name
 - layer to be deleted
 Output:
 */
static int l_map_delete_layer(lua_State* L)
{
	const char * map_name;
	int layer;
	int res;

	map_name = luaL_checkstring(L, -2);
	layer = luaL_checkint(L, -1);
	res = map_delete_layer(map_name, layer);
	lua_pushnumber(L, res);
	return 1;  // number of results
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
static int l_map_set_tile(lua_State* L)
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
	res = map_set_tile(map, layer, tile, x, y, true);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* map_set_tile_no_update

 Set a tile in a map without sending the resulting map to contexts
 Use map_broadcast to send the map

 Input:
 - ID of a map
 - layer of the map
 - ID of a tile
 - X coordinate of the tile to set
 - Y coordinate of the tile to set
 - Map level
 Output:
 */
static int l_map_set_tile_no_update(lua_State* L)
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
	res = map_set_tile(map, layer, tile, x, y, false);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* map_set_tile_array


 Input:
 - ID of a map
 - layer of the map
 - Array of tiles ID
 Output: -1 on error, 0 otherwise
 */
static int l_map_set_tile_array(lua_State* L)
{
	const char **arg = nullptr;
	int i;
	const char * map;
	int layer;

	map = luaL_checkstring(L, -3);
	layer = luaL_checkint(L, -2);

	i = 0;
	lua_pushnil(L);  // first key
	while (lua_next(L, -2) != 0)
	{
		// uses 'key' (at index -2) and 'value' (at index -1)
		arg = (const char**) realloc(arg, sizeof(char*) * (i + 2));
		arg[i] = luaL_checkstring(L, -1);
		i++;
		// removes 'value'; keeps 'key' for next iteration
		lua_pop(L, 1);
	}
	arg[i] = nullptr;

	map_set_tile_array(map, layer, arg);

	free(arg);
	lua_pushnumber(L, 0);
	return 1;  // number of results
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
static int l_map_set_tile_type(lua_State* L)
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
	res = map_set_tile_type(map, layer, type, x, y, true);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* map_set_tile_type_no_update

 Set a type in a map without sending the resulting map to contexts
 Use map_broadcast to send the map

 Input:
 - ID of a map
 - layer of the map
 - type
 - X coordinate of the tile to set
 - Y coordinate of the tile to set
 Output:
 */
static int l_map_set_tile_type_no_update(lua_State* L)
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
	res = map_set_tile_type(map, layer, type, x, y, true);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* map_broadcast

 Send map to all context.
 Call this function after map_*_no_update functions

 Input:
 - ID of a map
 Output:
 */
static int l_map_broadcast(lua_State* L)
{
	const char * map;
	int res = 0;

	map = luaL_checkstring(L, -1);
	map_broadcast(map);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* map_set_offscreen

 Set a map's layer off-screen

 Input:
 - ID of a map
 - off-screen script
 Output:
 */
static int l_map_set_offscreen(lua_State* L)
{
	const char * map;
	const char * script;
	int res;

	map = luaL_checkstring(L, -2);
	script = luaL_checkstring(L, -1);
	res = map_set_offscreen(map, script);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* map_set_custom_column

 Set a map's layer custom column tiling

 Input:
 - ID of a map
 - layer of the map
 - number of the column
 - width of column
 - height of column
 Output:
 */
static int l_map_set_custom_column(lua_State* L)
{
	const char * map;
	int layer;
	int num;
	int width;
	int height;
	int res;

	map = luaL_checkstring(L, -5);
	layer = luaL_checkint(L, -4);
	num = luaL_checkint(L, -3);
	width = luaL_checkint(L, -2);
	height = luaL_checkint(L, -1);
	res = map_set_custom_column(map, layer, num, width, height);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* map_set_custom_row

 Set a map's layer custom row tiling

 Input:
 - ID of a map
 - layer of the map
 - number of the row
 - width of row
 - height of row
 Output:
 */
static int l_map_set_custom_row(lua_State* L)
{
	const char * map;
	int layer;
	int num;
	int width;
	int height;
	int res;

	map = luaL_checkstring(L, -5);
	layer = luaL_checkint(L, -4);
	num = luaL_checkint(L, -3);
	width = luaL_checkint(L, -2);
	height = luaL_checkint(L, -1);
	res = map_set_custom_row(map, layer, num, width, height);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* print_text_id

 Send a message to a character

 Input:
 - ID of the receiver
 - message
 Output:
 */
static int l_print_text_id(lua_State* L)
{
	const char * id;
	const char * text;

	id = luaL_checkstring(L, -2);
	text = luaL_checkstring(L, -1);
	// add a trailing \n
	network_send_text(std::string(id), std::string(text));
	return 0;  // number of results
}

/* print_text_map

 Send a message to all characters on a map

 Input:
 - ID of a map
 - message
 Output:
 */
static int l_print_text_map(lua_State* L)
{
	const char * map;
	const char * string;

	map = luaL_checkstring(L, -2);
	string = luaL_checkstring(L, -1);
	// add a trailing \n
	context_broadcast_text(std::string(map), string);
	return 0;  // number of results
}

/* print_text_server

 Send a message to all characters on the server

 Input:
 - message
 Output:
 */
static int l_print_text_server(lua_State* L)
{
	const char * string;

	string = luaL_checkstring(L, -1);
	// add a trailing \n
	context_broadcast_text("", string);
	return 0;  // number of results
}

/* print_text_debug

 Print a message in the server's log (mainly for debug purpose)

 Input:
 - message
 Output:
 */
static int l_print_text_debug(lua_State* L)
{
	const char * string;

	string = luaL_checkstring(L, -1);
	wlog(LOGDESIGNER, (char* ) string);
	return 0;  // number of results
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
static int l_map_add_item(lua_State* L)
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
	res = map_add_item(map, layer, item, x, y);
	lua_pushnumber(L, res);
	return 1;  // number of results
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
static int l_map_delete_item(lua_State* L)
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
	res = map_delete_item(map, layer, x, y);
	lua_pushstring(L, res);
	if (res)
	{
		free(res);
	}
	return 1;  // number of results
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
static int l_map_add_event(lua_State* L)
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
	res = map_add_event(map, layer, script, x, y);
	lua_pushstring(L, res);
	if (res)
	{
		free(res);
	}
	return 1;  // number of results
}

/* map_add_event_param

 Add a parameter to the given event

 Input:
 - ID of a map
 - Layer of the map
 - ID of an event
 - parameter to add
 Output:
 -1 on error
 0 on success
 */
static int l_map_add_event_param(lua_State* L)
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
	res = map_add_event_param(map, layer, event_id, param);
	lua_pushnumber(L, res);
	return 1;  // number of results
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
static int l_map_delete_event(lua_State* L)
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
	res = map_delete_event(map, layer, script, x, y);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* map_add_scenery

 Add a scenery on a map

 Input:
 - ID of a map
 - Layer of the map
 - X coordinate in the map in pixel
 - Y coordinate in the map in pixel
 - image file name
 Output: Event ID
 */
static int l_map_add_scenery(lua_State* L)
{
	const char * map;
	int layer;
	int x;
	int y;
	const char * image_name;
	char *res;

	map = luaL_checkstring(L, -5);
	layer = luaL_checkint(L, -4);
	x = luaL_checkint(L, -3);
	y = luaL_checkint(L, -2);
	image_name = luaL_checkstring(L, -1);
	res = map_add_scenery(map, layer, x, y, image_name);
	lua_pushstring(L, res);
	if (res)
	{
		free(res);
	}
	return 1;  // number of results
}

/* map_effect

 Send map effect to all context on the same map.

 Input:
 - ID of a map
 - Array of string describing the effect
 Output: -1 on error, 0 otherwise
 */
static int l_map_effect(lua_State* L)
{
	const int l_NumArg = lua_gettop(L);

	std::string l_Target = luaL_checkstring(L, -l_NumArg);

	std::vector<std::string> l_Param;
	for (int l_Idx = 1; l_Idx < l_NumArg; l_Idx++) // 1 because 0 is the target
	{
		l_Param.push_back(luaL_checkstring(L, -l_NumArg + l_Idx));
	}

	network_broadcast_effect(EffectManager::EffectType::MAP, l_Target, l_Param);

	lua_pushnumber(L, 0);
	return 1;  // number of results
}

/* tile_get_x
 Input:
 - X coordinate (in tiles)
 - Y coordinate (in tiles)
 Output: X coordinate of the tile in pixels
 */
static int l_tile_get_x(lua_State* L)
{
	const char * map;
	int layer;
	int x;
	int y;
	int res = -1;

	map = luaL_checkstring(L, -4);
	layer = luaL_checkint(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	map_get_tile_coord(map, layer, x, y, &res, nullptr);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* tile_get_y
 Input:
 - X coordinate (in tiles)
 - Y coordinate (in tiles)
 Output: Y coordinate of the tile in pixels
 */
static int l_tile_get_y(lua_State* L)
{
	const char * map;
	int layer;
	int x;
	int y;
	int res = -1;

	map = luaL_checkstring(L, -4);
	layer = luaL_checkint(L, -3);
	x = luaL_checkint(L, -2);
	y = luaL_checkint(L, -1);
	map_get_tile_coord(map, layer, x, y, nullptr, &res);
	lua_pushnumber(L, res);
	return 1;  // number of results
}
/* inventory_delete

 Delete an item from the inventory

 Input:
 - ID of a character
 - ID of an item
 Output:
 */
static int l_inventory_delete(lua_State* L)
{
	const char * id;
	const char * item;
	int res;

	id = luaL_checkstring(L, -2);
	item = luaL_checkstring(L, -1);
	res = inventory_delete(id, item);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* inventory_add

 Add an item in the inventory

 Input:
 - ID of a character
 - ID of an item
 Output:
 */
static int l_inventory_add(lua_State* L)
{
	const char * id;
	const char * item;
	int res;

	id = luaL_checkstring(L, -2);
	item = luaL_checkstring(L, -1);
	res = inventory_add(id, item);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* inventory_get_by_name

 Get item ID from its name

 Input:
 - ID of a character
 - name of an item
 Output: ID of an item of that type
 */
static int l_inventory_get_by_name(lua_State* L)
{
	const char * id;
	const char * item_name;
	char * res;

	id = luaL_checkstring(L, -2);
	item_name = luaL_checkstring(L, -1);
	res = inventory_get_by_name(id, item_name);
	lua_pushstring(L, res);
	if (res)
	{
		free(res);
	}
	return 1;  // number of results
}

/* item_create_empty

 Create an empty item

 Input:
 Output: ID of a new item
 */
static int l_item_create_empty(lua_State* L)
{
	const std::pair<bool, std::string> res = item_create_empty();

	lua_pushstring(L, res.second.c_str());

	return 1;  // number of results
}

/* item_create_from_template

 Create an item from a template

 Input: item template name
 Output: ID of a new item
 */
static int l_item_create_from_template(lua_State* L)
{
	const char * item;

	item = luaL_checkstring(L, -1);
	const std::pair<bool, std::string> res = item_create_from_template(std::string(item));
	lua_pushstring(L, res.second.c_str());

	return 1;  // number of results
}

/* resource_new

 Create a new resource

 Input:
 - Name of the template
 - Quantity
 Output: ID of the new resource
 */
static int l_resource_new(lua_State* L)
{
	const std::string mytemplate(luaL_checkstring(L, -2));
	int quantity = luaL_checkint(L, -1);

	const std::pair<bool, std::string> resource = resource_new(mytemplate, quantity);
	lua_pushstring(L, resource.second.c_str());

	return 1;  // number of results
}

/* resource_get_quantity

 Get the quantity of a resource

 Input:
 - ID of a resource
 Output: Quantity of that resource
 */
static int l_resource_get_quantity(lua_State* L)
{
	const char * resource;
	int res;

	resource = luaL_checkstring(L, -1);
	res = resource_get_quantity(std::string(resource));
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* resource_set_quantity

 Set the quantity of a resource

 Input:
 - ID of a resource
 - quantity to set
 Output: -1 on error
 */
static int l_resource_set_quantity(lua_State* L)
{
	const char * resource;
	int quantity;
	int res;
	Context * context;

	lua_getglobal(L, LUAVM_CONTEXT);
	context = (Context*) lua_touserdata(L, -1);
	lua_pop(L, 1);

	resource = luaL_checkstring(L, -2);
	quantity = luaL_checkint(L, -1);
	res = resource_set_quantity(context, std::string(resource), quantity);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* item_destroy

 Delete an item

 Input:
 - ID of an item
 Output:
 -1 on error
 0 on success
 */
static int l_item_destroy(lua_State* L)
{
	const char * item;
	int res;

	item = luaL_checkstring(L, -1);
	res = item_destroy(std::string(item));
	lua_pushnumber(L, res);
	return 1;  // number of results
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
static int l_map_get_tile(lua_State* L)
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
	res = map_get_tile(map, layer, x, y);
	lua_pushstring(L, res);
	if (res)
	{
		free(res);
	}
	return 1;  // number of results
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
static int l_map_get_tile_type(lua_State* L)
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
	res = map_get_tile_type(std::string(map), layer, x, y);
	lua_pushstring(L, res);
	free(res);
	return 1;  // number of results
}

/* map_get_character

 Get list of characters on a tile

 Input:
 - ID of a map
 - X coordinate (in tiles)
 - Y coordinate (in tiles)
 Output: Array of characters on that tile
 */
static int l_map_get_character(lua_State* L)
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

	res = map_get_character(std::string(map), x, y);
	if (res)
	{
		cur_res = res;
		while (*cur_res != nullptr)
		{
			lua_pushstring(L, *cur_res);
			free(*cur_res);
			res_num++;
			cur_res++;
		}
		free(res);
	}

	return res_num;  // number of results
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
static int l_map_get_item(lua_State* L)
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

	res = map_get_item(map, layer, x, y);
	if (res)
	{
		cur_res = res;
		while (*cur_res != nullptr)
		{
			lua_pushstring(L, *cur_res);
			free(*cur_res);
			res_num++;
			cur_res++;
		}
		free(res);
	}

	return res_num;  // number of results
}

/* character_attribute_change

 Add (or remove) a value to the given attribute

 Input:
 - ID of a character
 - ID of an attribute
 - value to add (may be negative)
 Output:
 */
static int l_character_attribute_change(lua_State* L)
{
	const char * id;
	const char * attribute;
	int value;
	int res;
	Context * context;

	id = luaL_checkstring(L, -3);
	attribute = luaL_checkstring(L, -2);
	value = luaL_checkint(L, -1);

	lua_getglobal(L, LUAVM_CONTEXT);
	context = (Context*) lua_touserdata(L, -1);
	lua_pop(L, 1);

	res = attribute_change(context, CHARACTER_TABLE, id, attribute, value);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* character_attribute_get

 Get the value of the given attribute

 Input:
 - ID of a character
 - ID of an attribute
 Output: Value of the given attribute or -1 if error
 */
static int l_character_attribute_get(lua_State* L)
{
	const char * id;
	const char * attribute;
	int res;

	id = luaL_checkstring(L, -2);
	attribute = luaL_checkstring(L, -1);
	res = attribute_get(CHARACTER_TABLE, id, attribute);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* character_attribute_set

 Set an attribute to the given value

 Input:
 - ID of a character
 - ID of an attribute
 - value
 Output:
 */
static int l_character_attribute_set(lua_State* L)
{
	const char * id;
	const char * attribute;
	int value;
	int res;

	id = luaL_checkstring(L, -3);
	attribute = luaL_checkstring(L, -2);
	value = luaL_checkint(L, -1);
	res = attribute_set(CHARACTER_TABLE, id, attribute, value);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* character_attribute_tag_get

 Get the value of the given attribute tag

 Input:
 - ID of a character
 - ID of an attribute
 Output: Value of the given attribute or -1 if error
 */
static int l_character_attribute_tag_get(lua_State* L)
{
	const char * id;
	const char * attribute;
	char * res;

	id = luaL_checkstring(L, -2);
	attribute = luaL_checkstring(L, -1);
	res = attribute_tag_get(CHARACTER_TABLE, id, attribute);
	lua_pushstring(L, res);
	if (res)
	{
		free(res);
	}
	return 1;  // number of results
}

/* character_attribute_tag_set

 Set an attribute tag to the given value

 Input:
 - ID of a character
 - ID of an attribute
 - value
 Output:
 */
static int l_character_attribute_tag_set(lua_State* L)
{
	const char * id;
	const char * attribute;
	const char * value;
	int res;

	id = luaL_checkstring(L, -3);
	attribute = luaL_checkstring(L, -2);
	value = luaL_checkstring(L, -1);
	res = attribute_tag_set(CHARACTER_TABLE, id, attribute, value);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* map_attribute_change

 Add (or remove) a value to the given attribute

 Input:
 - ID of a map
 - ID of an attribute
 - value to add (may be negative)
 Output:
 */
static int l_map_attribute_change(lua_State* L)
{
	const char * id;
	const char * attribute;
	int value;
	int res;
	Context * context;

	id = luaL_checkstring(L, -3);
	attribute = luaL_checkstring(L, -2);
	value = luaL_checkint(L, -1);

	lua_getglobal(L, LUAVM_CONTEXT);
	context = (Context*) lua_touserdata(L, -1);
	lua_pop(L, 1);

	res = attribute_change(context, MAP_TABLE, id, attribute, value);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* map_attribute_get

 Get the value of the given attribute

 Input:
 - ID of a map
 - ID of an attribute
 Output: Value of the given attribute
 */
static int l_map_attribute_get(lua_State* L)
{
	const char * id;
	const char * attribute;
	int res;

	id = luaL_checkstring(L, -2);
	attribute = luaL_checkstring(L, -1);
	res = attribute_get(MAP_TABLE, id, attribute);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* map_attribute_set

 Set an attribute to the given value

 Input:
 - ID of a map
 - ID of an attribute
 - value
 Output:
 */
static int l_map_attribute_set(lua_State* L)
{
	const char * id;
	const char * attribute;
	int value;
	int res;

	id = luaL_checkstring(L, -3);
	attribute = luaL_checkstring(L, -2);
	value = luaL_checkint(L, -1);
	res = attribute_set(MAP_TABLE, id, attribute, value);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* equipment_slot_add_item

 Add an item to an equipment slot

 Input:
 - ID of a character
 - ID of an equipment slot
 - ID of an item
 Output:
 */
static int l_equipment_slot_set_item(lua_State* L)
{
	const char * id;
	const char * slot;
	const char * item;
	int res;

	id = luaL_checkstring(L, -3);
	slot = luaL_checkstring(L, -2);
	item = luaL_checkstring(L, -1);
	res = equipment_set_item(id, slot, item);
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/* equipment_slot_get_item

 Get the item's ID of an equipment slot

 Input:
 - ID of a character
 - ID of an equipment slot
 Output: ID of an item
 */
static int l_equipment_slot_get_item(lua_State* L)
{
	const char * id;
	const char * slot;
	char * item;

	id = luaL_checkstring(L, -2);
	slot = luaL_checkstring(L, -1);
	item = equipment_get_item(id, slot);
	if (item == nullptr)
	{
		return 0;  // number of results
	}
	lua_pushstring(L, item);
	free(item);
	return 1;  // number of results
}

/* get_base_directory
 Input:
 Output: input data base directory
 */
static int l_get_base_directory(lua_State* L)
{
	lua_pushstring(L, base_directory.c_str());
	return 1;  // number of results
}

/* popup_send

 Send a popup screen to a context

 Input:
 - ID of the target character
 - Array of string describing the pop-up
 Output: -1 on error, 0 otherwise
 */
static int l_popup_send(lua_State* L)
{
	int l_NumArg = lua_gettop(L);

	std::vector<std::string> l_PopupData;

	const char * l_Id = luaL_checkstring(L, -l_NumArg + 0);

	for (int l_Index = 1; l_Index < l_NumArg; l_Index++)
	{
		l_PopupData.push_back(luaL_checkstring(L, -l_NumArg + l_Index));
	}

	network_send_popup(l_Id, l_PopupData);

	lua_pushnumber(L, 0);
	return 1;  // number of results
}

/**************************************
 **************************************/
static void action_chat(Context * context, const std::string & text)
{
	const std::string new_text = context->getCharacterName() + ":" + text;

	network_broadcast_text(context, new_text);
}

/**************************************
 Execute a LUA script file
 return -1 if the script do not return something
 **************************************/
int action_execute_script(Context * context, const char * script, const char ** parameters)
{
	if (script == nullptr)
	{
		return -1;
	}

	// Special case for chat
	if (strcmp(script, WOG_CHAT) == 0)
	{
		action_chat(context, std::string(parameters[0]));
		return -1;
	}

	return lua_execute_script(context->getLuaVm(), script, parameters);
}

/**************************************
 Execute an action configuration file
 return -1 if the script do not return something
 **************************************/
int action_execute(Context * context, const char * action, char ** parameters)
{
	char * script = nullptr;
	char ** params = nullptr;
	char ** all_params = nullptr;
	int ret = -1;

	if (entry_read_string(ACTION_TABLE, action, &script, ACTION_KEY_SCRIPT, nullptr) == false)
	{
		return -1;
	}

	entry_read_list(ACTION_TABLE, action, &params, ACTION_KEY_PARAM, nullptr);

	all_params = add_array(params, parameters);

	ret = action_execute_script(context, script, (const char**) all_params);

	deep_free(params);
	free(all_params);

	return ret;
}

/**************************************
 Execute an action configuration file
 return -1 if the script do not return something
 **************************************/
int action_execute(Context * context, const std::string & actionName, const std::vector<std::string> & parameters)
{
	char * script = nullptr;

	if (entry_read_string(ACTION_TABLE, actionName.c_str(), &script,
	ACTION_KEY_SCRIPT, nullptr) == false)
	{
		ERR_DESIGN
		return -1;
	}

	char ** params = nullptr;

	entry_read_list(ACTION_TABLE, actionName.c_str(), &params,
	ACTION_KEY_PARAM, nullptr);

	char ** passed_param = to_array(parameters);

	char ** all_params = add_array(params, passed_param);

	int ret = action_execute_script(context, script, (const char**) all_params);

	free(script);

	deep_free(params);
	deep_free(passed_param);
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
static int l_call_script(lua_State* L)
{
	const char * script;
	int num_arg;
	char **arg = nullptr;
	int i;
	int res;
	Context * context;

	lua_getglobal(L, LUAVM_CONTEXT);
	context = (Context*) lua_touserdata(L, -1);
	lua_pop(L, 1);

	num_arg = lua_gettop(L);
	script = luaL_checkstring(L, -num_arg);
	if (num_arg > 1)
	{
		arg = (char**) malloc(sizeof(char*) * num_arg);
		for (i = 0; i < num_arg - 1; i++)
		{
			arg[i] = (char *) luaL_checkstring(L, -num_arg + 1 + i); // FIXME wrong casting ?
		}
		arg[i] = nullptr; // End of list
	}

	res = action_execute_script(context, script, (const char**) arg);

	if (arg)
	{
		free(arg);
	}
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/***************************************************
 Call an action

 Input:
 - action name
 - Array of parameters
 Output: -1 on script error, if no error script return value
 ***************************************************/
static int l_call_action(lua_State* L)
{
	const char * action;
	int num_arg;
	char **arg = nullptr;
	int i;
	int res;
	Context * context;

	lua_getglobal(L, LUAVM_CONTEXT);
	context = (Context*) lua_touserdata(L, -1);
	lua_pop(L, 1);

	num_arg = lua_gettop(L);
	action = luaL_checkstring(L, -num_arg);
	if (num_arg > 1)
	{
		arg = (char**) malloc(sizeof(char*) * num_arg);
		for (i = 0; i < num_arg - 1; i++)
		{
			arg[i] = (char *) luaL_checkstring(L, -num_arg + 1 + i); // FIXME wrong casting ?
		}
		arg[i] = nullptr; // End of list
	}

	res = action_execute(context, action, arg);

	if (arg)
	{
		free(arg);
	}
	lua_pushnumber(L, res);
	return 1;  // number of results
}

/***************************************************
 ***************************************************/
void register_lua_functions(Context * context)
{
	wlog(LOGDEVELOPER, "Registering LUA functions");

	lua_State* L = context->getLuaVm();

	// player functions
	lua_pushcfunction(L, l_player_get_id);
	lua_setglobal(L, "player_get_id");
	// character functions
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
	lua_pushcfunction(L, l_character_set_ai_script);
	lua_setglobal(L, "character_set_ai_script");
	lua_pushcfunction(L, l_character_set_sprite);
	lua_setglobal(L, "character_set_sprite");
	lua_pushcfunction(L, l_character_set_sprite_dir);
	lua_setglobal(L, "character_set_sprite_dir");
	lua_pushcfunction(L, l_character_set_sprite_move);
	lua_setglobal(L, "character_set_sprite_move");
	lua_pushcfunction(L, l_character_wake_up);
	lua_setglobal(L, "character_wake_up");
	lua_pushcfunction(L, l_character_out_of_game);
	lua_setglobal(L, "character_out_of_game");
	lua_pushcfunction(L, l_character_disconnect);
	lua_setglobal(L, "character_disconnect");
	lua_pushcfunction(L, l_character_delete);
	lua_setglobal(L, "character_delete");
	lua_pushcfunction(L, l_character_broadcast);
	lua_setglobal(L, "character_broadcast");
	lua_pushcfunction(L, l_character_effect);
	lua_setglobal(L, "character_effect");
	// map functions
	lua_pushcfunction(L, l_map_new);
	lua_setglobal(L, "map_new");
	lua_pushcfunction(L, l_map_add_layer);
	lua_setglobal(L, "map_add_layer");
	lua_pushcfunction(L, l_map_delete_layer);
	lua_setglobal(L, "map_delete_layer");
	lua_pushcfunction(L, l_map_set_tile);
	lua_setglobal(L, "map_set_tile");
	lua_pushcfunction(L, l_map_set_tile_no_update);
	lua_setglobal(L, "map_set_tile_no_update");
	lua_pushcfunction(L, l_map_set_tile_array);
	lua_setglobal(L, "map_set_tile_array");
	lua_pushcfunction(L, l_map_set_tile_type);
	lua_setglobal(L, "map_set_tile_type");
	lua_pushcfunction(L, l_map_set_tile_type_no_update);
	lua_setglobal(L, "map_set_tile_type_no_update");
	lua_pushcfunction(L, l_map_broadcast);
	lua_setglobal(L, "map_broadcast");
	lua_pushcfunction(L, l_map_set_offscreen);
	lua_setglobal(L, "map_set_offscreen");
	lua_pushcfunction(L, l_map_set_custom_column);
	lua_setglobal(L, "map_set_custom_column");
	lua_pushcfunction(L, l_map_set_custom_row);
	lua_setglobal(L, "map_set_custom_row");
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
	lua_pushcfunction(L, l_map_add_scenery);
	lua_setglobal(L, "map_add_scenery");
	lua_pushcfunction(L, l_map_effect);
	lua_setglobal(L, "map_effect");
	// tile functions
	lua_pushcfunction(L, l_tile_get_x);
	lua_setglobal(L, "tile_get_x");
	lua_pushcfunction(L, l_tile_get_y);
	lua_setglobal(L, "tile_get_y");
	// inventory functions
	lua_pushcfunction(L, l_inventory_delete);
	lua_setglobal(L, "inventory_delete");
	lua_pushcfunction(L, l_inventory_add);
	lua_setglobal(L, "inventory_add");
	lua_pushcfunction(L, l_inventory_get_by_name);
	lua_setglobal(L, "inventory_get_by_name");
	// item functions
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
	// character attribute functions
	lua_pushcfunction(L, l_character_attribute_change);
	lua_setglobal(L, "character_attribute_change");
	lua_pushcfunction(L, l_character_attribute_get);
	lua_setglobal(L, "character_attribute_get");
	lua_pushcfunction(L, l_character_attribute_set);
	lua_setglobal(L, "character_attribute_set");
	lua_pushcfunction(L, l_character_attribute_tag_get);
	lua_setglobal(L, "character_attribute_tag_get");
	lua_pushcfunction(L, l_character_attribute_tag_set);
	lua_setglobal(L, "character_attribute_tag_set");
	// map attribute functions
	lua_pushcfunction(L, l_map_attribute_change);
	lua_setglobal(L, "map_attribute_change");
	lua_pushcfunction(L, l_map_attribute_get);
	lua_setglobal(L, "map_attribute_get");
	lua_pushcfunction(L, l_map_attribute_set);
	lua_setglobal(L, "map_attribute_set");
	// equipment functions
	lua_pushcfunction(L, l_equipment_slot_set_item);
	lua_setglobal(L, "equipment_slot_set_item");
	lua_pushcfunction(L, l_equipment_slot_get_item);
	lua_setglobal(L, "equipment_slot_get_item");
	// Miscellaneous functions
	lua_pushcfunction(L, l_get_base_directory);
	lua_setglobal(L, "get_base_directory");
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

	// push the context on LUA VM stack
	lua_pushlightuserdata(L, context);
	lua_setglobal(L, LUAVM_CONTEXT);
}
