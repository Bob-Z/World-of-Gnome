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

#include "map_server.h"

#include <const.h>
#include "Context.h"
#include <cstdio>
#include <cstring>
#include <entry.h>
#include <file.h>
#include <map.h>
#include <mutex.h>
#include <SDL_mutex.h>
#include <stdlib.h>
#include <syntax.h>
#include <util.h>

#include "action.h"
#include "context_server.h"

/***********************************
 return string MUST BE FREED
 ***********************************/
static char * get_tile_type_through_layer(const std::string & map, int layer, int x, int y)
{
	char * type;
	while (layer >= 0)
	{
		type = map_get_tile_type(map, layer, x, y);
		if (type)
		{
			return type;
		}
		layer--;
	}

	return nullptr;
}

/***********************************
 check if id is allowed to go on a tile
 return 1 if the context is allowed to go to the tile at coord x,y
 return 0 if the context is NOT allowed to go to the tile at coord x,y
 return false on error or no data found
 *************************************/
int map_check_tile(Context * ctx, const char * id, const std::string & map, int layer, int x, int y)
{
	char * script;
	char sx[64];
	char sy[64];
	int res;
	char * tile_type;
	char ** allowed_tile;
	int i = 0;
	int width = 0;
	int height = 0;

	if (entry_read_int(MAP_TABLE, map.c_str(), &width, MAP_KEY_WIDTH, nullptr) == false)
	{
		return false;
	}
	if (entry_read_int(MAP_TABLE, map.c_str(), &height, MAP_KEY_HEIGHT, nullptr) == false)
	{
		return false;
	}

	if (x < 0 || y < 0 || x >= width || y >= height)
	{
		return 0;
	}

	// If there is an allowed_tile_script, run it
	if (entry_read_string(CHARACTER_TABLE, id, &script,
	CHARACTER_KEY_ALLOWED_TILE_SCRIPT, nullptr) == true)
	{
		const char * param[5];

		param[0] = id;
		param[1] = map.c_str();
		sprintf(sx, "%d", x);
		sprintf(sy, "%d", y);
		param[2] = sx;
		param[3] = sy;
		param[4] = nullptr;
		res = action_execute_script(ctx, script, (const char**) param);
		free(script);
		return res;
	}

	// Read tile at given index on this map
	entry_read_int(CHARACTER_TABLE, id, &layer, CHARACTER_LAYER, nullptr);
	tile_type = get_tile_type_through_layer(map, layer, x, y);

	// Allow tile if no type defined
	if (tile_type == nullptr)
	{
		return 1;
	}

	// Allow tile if its type is empty (i.e. "")
	if (tile_type[0] == 0)
	{
		free(tile_type);
		return 1;
	}

	// If there is allowed_tile list, check it
	if (entry_read_list(CHARACTER_TABLE, id, &allowed_tile,
	CHARACTER_KEY_ALLOWED_TILE, nullptr) == true)
	{
		i = 0;
		while (allowed_tile[i] != nullptr)
		{
			if (strcmp(allowed_tile[i], tile_type) == 0)
			{
				deep_free(allowed_tile);
				free(tile_type);
				return 1;
			}
			i++;
		}

		deep_free(allowed_tile);
		free(tile_type);
		return 0;
	}

	free(tile_type);
	// Allow all tiles by default
	return 1;
}

/***********************************
 Create a new map or add a map layer.
 Return the name of the new map
 Return string must be freed by caller
 *************************************/
std::pair<bool, std::string> map_new(const char *name, int w, int h, int tile_w, int tile_h)
{
	if (w < 0 || h < 0)
	{
		return std::pair<bool, std::string>
		{ false, "" };
	}

	std::pair<bool, std::string> map_name = file_new(MAP_TABLE, std::string(name));
	if (map_name.first == false)
	{
		return map_name;
	}

	if (entry_write_int(MAP_TABLE, map_name.second.c_str(), w, MAP_KEY_HEIGHT, nullptr) == false)
	{
		return std::pair<bool, std::string>
		{ false, "" };
	}
	if (entry_write_int(MAP_TABLE, map_name.second.c_str(), h, MAP_KEY_WIDTH, nullptr) == false)
	{
		return std::pair<bool, std::string>
		{ false, "" };
	}
	if (entry_write_int(MAP_TABLE, map_name.second.c_str(), tile_w, MAP_KEY_TILE_WIDTH, nullptr) == false)
	{
		return std::pair<bool, std::string>
		{ false, "" };
	}
	if (entry_write_int(MAP_TABLE, map_name.second.c_str(), tile_h, MAP_KEY_TILE_HEIGHT, nullptr) == false)
	{
		return std::pair<bool, std::string>
		{ false, "" };
	}

	return map_name;
}

/**************************************
 delete an item on context's map
 **************************************/
char * map_delete_item(const char * map, int layer, int x, int y)
{
	char ** itemlist;
	int i = 0;
	int mapx;
	int mapy;
	const char * id = nullptr;
	char * saved_item = nullptr;
	char layer_name[SMALL_BUF];

	if (x < 0 || y < 0)
	{
		return nullptr;
	}

	sprintf(layer_name, "%s%d", MAP_KEY_LAYER, layer);
	/* Manage concurrent acces to map files */
	SDL_LockMutex(map_mutex);
	/* Search the items on the specified tile */
	if (entry_get_group_list(MAP_TABLE, map, &itemlist, layer_name,
	MAP_ENTRY_ITEM_LIST, nullptr) == false)
	{
		SDL_UnlockMutex(map_mutex);
		return nullptr;
	}

	while (itemlist[i] != nullptr)
	{
		if (entry_read_int(MAP_TABLE, map, &mapx, layer_name,
		MAP_ENTRY_ITEM_LIST, itemlist[i], MAP_ITEM_TILE_X, nullptr) == false)
		{
			SDL_UnlockMutex(map_mutex);
			deep_free(itemlist);
			return nullptr;
		}

		if (entry_read_int(MAP_TABLE, map, &mapy, layer_name,
		MAP_ENTRY_ITEM_LIST, itemlist[i], MAP_ITEM_TILE_Y, nullptr) == false)
		{
			SDL_UnlockMutex(map_mutex);
			deep_free(itemlist);
			return nullptr;
		}

		if (x == mapx && y == mapy)
		{
			id = itemlist[i];
			saved_item = strdup(itemlist[i]);
			break;
		}

		i++;
	}

	if (id == nullptr)
	{
		deep_free(itemlist);
		if (saved_item)
		{
			free(saved_item);
		}
		SDL_UnlockMutex(map_mutex);
		return nullptr;
	}

	/* remove the item from the item list of the map */
	if (entry_remove_group(MAP_TABLE, map, id, layer_name, MAP_ENTRY_ITEM_LIST, nullptr) == false)
	{
		deep_free(itemlist);
		free(saved_item);
		SDL_UnlockMutex(map_mutex);
		return nullptr;
	}

	deep_free(itemlist);

	SDL_UnlockMutex(map_mutex);

	/* Send network notifications */
	context_broadcast_map(map);

	return saved_item;
}

/******************************************
 Add an item on map at given coordinate
 return false if fails
 ******************************************/
int map_add_item(const char * map, int layer, const char * id, int x, int y)
{
	char layer_name[SMALL_BUF];

	if (x < 0 || y < 0)
	{
		return false;
	}

	sprintf(layer_name, "%s%d", MAP_KEY_LAYER, layer);

	SDL_LockMutex(map_mutex);

	if (entry_write_int(MAP_TABLE, map, x, layer_name, MAP_ENTRY_ITEM_LIST, id,
	MAP_ITEM_TILE_X, nullptr) == false)
	{
		entry_remove_group(MAP_TABLE, map, id, layer_name, MAP_ENTRY_ITEM_LIST, nullptr);
		SDL_UnlockMutex(map_mutex);
		return false;
	}
	if (entry_write_int(MAP_TABLE, map, y, layer_name, MAP_ENTRY_ITEM_LIST, id,
	MAP_ITEM_TILE_Y, nullptr) == false)
	{
		entry_remove_group(MAP_TABLE, map, id, layer_name, MAP_ENTRY_ITEM_LIST, nullptr);
		SDL_UnlockMutex(map_mutex);
		return false;
	}

	SDL_UnlockMutex(map_mutex);

	// Send network notifications
	context_broadcast_map(map);

	return true;
}

/***********************************
 Write a new tile into a map set
 return false if fails
 ***********************************/
int map_set_tile(const char * map, int layer, const char * tile, int x, int y, int network_broadcast)
{
	char * previous_tile = nullptr;
	int width = -1;
	int index;
	char layer_name[SMALL_BUF];

	if (map == nullptr || tile == nullptr)
	{
		return false;
	}

	if (x < 0 || y < 0)
	{
		return false;
	}

	sprintf(layer_name, "%s%d", MAP_KEY_LAYER, layer);

	/* Manage concurrent access to map files */
	SDL_LockMutex(map_mutex);

	/* read size of map grid */
	if (entry_read_int(MAP_TABLE, map, &width, MAP_KEY_WIDTH, nullptr) == false)
	{
		SDL_UnlockMutex(map_mutex);
		return false;
	}

	/* read layer's specific width */
	entry_read_int(MAP_TABLE, map, &width, layer_name, MAP_KEY_WIDTH, nullptr);

	index = width * y + x;

	/* read previous map set */
	if (entry_read_list_index(MAP_TABLE, map, &previous_tile, index, layer_name,
	MAP_KEY_SET, nullptr) == false)
	{
		SDL_UnlockMutex(map_mutex);
		return false;
	}

	/* Do not change the tile if it is already the requested tile
	 Avoid calling useless context_broadcast_file */
	if (strcmp(previous_tile, tile) == 0)
	{
		free(previous_tile);
		SDL_UnlockMutex(map_mutex);
		return false;
	}
	free(previous_tile);

	if (entry_write_list_index(MAP_TABLE, map, tile, index, layer_name,
	MAP_KEY_SET, nullptr) == true)
	{
		if (network_broadcast)
		{
			context_broadcast_map(map);
		}
	}

	SDL_UnlockMutex(map_mutex);

	return true;
}

/***********************************
 Write an array of tiles into a map set layer
 tile_array is a nullptr teminated tiles ID
 return false if fails
 ***********************************/
int map_set_tile_array(const char * map, int layer, const char** tile_array)
{
	char ** previous_tile = nullptr;
	char ** current_tile = nullptr;
	char layer_name[SMALL_BUF];

	if (map == nullptr)
	{
		return false;
	}

	sprintf(layer_name, "%s%d", MAP_KEY_LAYER, layer);

	/* Manage concurrent access to map files */
	SDL_LockMutex(map_mutex);

	/* read previous map set */
	if (entry_read_list(MAP_TABLE, map, &previous_tile, layer_name, MAP_KEY_SET, nullptr) == false)
	{
		SDL_UnlockMutex(map_mutex);
		return false;
	}

	current_tile = previous_tile;
	while (*tile_array)
	{
		free(*current_tile);
		*current_tile = strdup(*tile_array);
		current_tile++;
		tile_array++;
	}

	if (entry_write_list(MAP_TABLE, map, previous_tile, layer_name, MAP_KEY_SET, nullptr) == true)
	{
		context_broadcast_map(map);
	}

	deep_free(previous_tile);

	SDL_UnlockMutex(map_mutex);

	return true;
}

/***********************************
 Write a new tile type into a map file
 return false if fails
 ***********************************/
int map_set_tile_type(const char * map, int layer, const char * type, int x, int y, int network_broadcast)
{
	char * previous_type = nullptr;
	int width = -1;
	int index;
	char layer_name[SMALL_BUF];

	if (map == nullptr || type == nullptr)
	{
		return false;
	}

	if (x < 0 || y < 0)
	{
		return false;
	}

	sprintf(layer_name, "%s%d", MAP_KEY_LAYER, layer);

	// Manage concurrent access to map files
	SDL_LockMutex(map_mutex);

	// read size of map grid
	if (entry_read_int(MAP_TABLE, map, &width, MAP_KEY_WIDTH, nullptr) == false)
	{
		SDL_UnlockMutex(map_mutex);
		return false;
	}

	index = width * y + x;

	// read previous map type
	if (entry_read_list_index(MAP_TABLE, map, &previous_type, index, layer_name,
	MAP_KEY_TYPE, nullptr) == true)
	{
		/* Do not change the type if it already the requested type
		 Avoid calling useless context_broadcast_file */
		if (strcmp(previous_type, type) == 0)
		{
			free(previous_type);
			SDL_UnlockMutex(map_mutex);
			return true;
		}
		free(previous_type);
	}

	if (entry_write_list_index(MAP_TABLE, map, type, index, layer_name,
	MAP_KEY_TYPE, nullptr) == true)
	{
		if (network_broadcast)
		{
			context_broadcast_map(map);
		}
	}

	SDL_UnlockMutex(map_mutex);

	return true;
}

/***********************************
 Broadcast map file to other context
 ***********************************/
void map_broadcast(const char * map)
{
	context_broadcast_map(map);
}

/***********************************
 Set offscreen script of a map
 return false if fails
 ***********************************/
int map_set_offscreen(const char * map, const char * script)
{
	int res;

	if (map == nullptr || script == nullptr)
	{
		return false;
	}

	/* Manage concurrent access to map files */
	SDL_LockMutex(map_mutex);

	res = entry_write_string(MAP_TABLE, map, script, MAP_OFFSCREEN, nullptr);

	SDL_UnlockMutex(map_mutex);

	return res;
}

/***********************************
 Set custom tiling of a map's layer
 if layer == -1, set the map's grid custom columns
 return false if fails
 ***********************************/
int map_set_custom_column(const char * map, int layer, int num, int width, int height)
{
	char layer_name[SMALL_BUF];
	char width_name[SMALL_BUF];
	char height_name[SMALL_BUF];
	int res;

	if (map == nullptr)
	{
		return false;
	}

	// Manage concurrent access to map files
	SDL_LockMutex(map_mutex);

	sprintf(width_name, "%s%d", MAP_KEY_COL_WIDTH, num);
	sprintf(height_name, "%s%d", MAP_KEY_COL_HEIGHT, num);

	// Map grid settings
	if (layer == -1)
	{
		res = entry_write_int(MAP_TABLE, map, width, width_name, nullptr);
		if (res == true)
		{
			res = entry_write_int(MAP_TABLE, map, height, height_name, nullptr);
		}
	}
	// Layer specific setting
	else
	{
		sprintf(layer_name, "%s%d", MAP_KEY_LAYER, layer);
		res = entry_write_int(MAP_TABLE, map, width, layer_name, width_name, nullptr);
		if (res == true)
		{
			res = entry_write_int(MAP_TABLE, map, height, layer_name, height_name, nullptr);
		}
	}

	SDL_UnlockMutex(map_mutex);

	return res;
}

/***********************************
 Set custom tiling of a map's layer
 if layer == -1, set the map's grid custom rows
 return false if fails
 ***********************************/
int map_set_custom_row(const char * map, int layer, int num, int width, int height)
{
	char layer_name[SMALL_BUF];
	char width_name[SMALL_BUF];
	char height_name[SMALL_BUF];
	int res;

	if (map == nullptr)
	{
		return false;
	}

	/* Manage concurrent access to map files */
	SDL_LockMutex(map_mutex);

	if (num == 0)
	{
		sprintf(width_name, "%s", MAP_KEY_ROW_WIDTH);
		sprintf(height_name, "%s", MAP_KEY_ROW_HEIGHT);
	}
	else
	{
		sprintf(width_name, "%s%d", MAP_KEY_ROW_WIDTH, num);
		sprintf(height_name, "%s%d", MAP_KEY_ROW_HEIGHT, num);
	}

	if (layer == -1)
	{
		res = entry_write_int(MAP_TABLE, map, width, width_name, nullptr);
		if (res == true)
		{
			res = entry_write_int(MAP_TABLE, map, height, height_name, nullptr);
		}
	}
	else
	{
		sprintf(layer_name, "%s%d", MAP_KEY_LAYER, layer);
		res = entry_write_int(MAP_TABLE, map, width, layer_name, width_name, nullptr);
		if (res == true)
		{
			res = entry_write_int(MAP_TABLE, map, height, layer_name, height_name, nullptr);
		}
	}

	SDL_UnlockMutex(map_mutex);

	return res;
}

/********************************************
 return the name of the tile on map at x,y
 return must be freed by caller
 ********************************************/
char * map_get_tile(const char * map, int layer, int x, int y)
{
	char * map_tile = nullptr;
	int width;
	int height;
	char layer_name[SMALL_BUF];

	sprintf(layer_name, "%s%d", MAP_KEY_LAYER, layer);

	if (entry_read_int(MAP_TABLE, map, &width, MAP_KEY_WIDTH, nullptr) == false)
	{
		return nullptr;
	}
	entry_read_int(MAP_TABLE, map, &width, layer_name, MAP_KEY_WIDTH, nullptr);

	if (entry_read_int(MAP_TABLE, map, &height, MAP_KEY_HEIGHT, nullptr) == false)
	{
		return nullptr;
	}
	entry_read_int(MAP_TABLE, map, &height, layer_name, MAP_KEY_HEIGHT, nullptr);

	if (x < 0 || y < 0 || x >= width || y >= height)
	{
		return nullptr;
	}

	entry_read_list_index(MAP_TABLE, map, &map_tile, (width * y) + x, layer_name, MAP_KEY_SET, nullptr);

	return map_tile;
}

/********************************************
 return the type of the tile on map layer at x,y
 Returned string MUST BE FREED
 ********************************************/
char * map_get_tile_type(const std::string & map, int layer, int x, int y)
{
	char * map_type = nullptr;
	int width;
	int height;
	char layer_name[SMALL_BUF];

	if (entry_read_int(MAP_TABLE, map.c_str(), &width, MAP_KEY_WIDTH, nullptr) == false)
	{
		return nullptr;
	}

	if (entry_read_int(MAP_TABLE, map.c_str(), &height, MAP_KEY_HEIGHT, nullptr) == false)
	{
		return nullptr;
	}

	if (x < 0 || y < 0 || x >= width || y >= height)
	{
		return nullptr;
	}

	sprintf(layer_name, "%s%d", MAP_KEY_LAYER, layer);
	entry_read_list_index(MAP_TABLE, map.c_str(), &map_type, (width * y) + x, layer_name, MAP_KEY_TYPE, nullptr);

	if (map_type != nullptr)
	{
		return map_type;
	}

	return map_type;
}

/************************************************
 return an array of event id on given map at x,y
 This array MUST be freed by caller
 ************************************************/
char ** map_get_event(const std::string & map, int layer, int x, int y)
{
	char ** eventlist = nullptr;
	char ** event_id = nullptr;
	int i = 0;
	int mapx;
	int mapy;
	int event_id_num = 0;
	char layer_name[SMALL_BUF];

	if (x < 0 || y < 0)
	{
		return nullptr;
	}

	sprintf(layer_name, "%s%d", MAP_KEY_LAYER, layer);

	// Manage concurrent acces to map files
	SDL_LockMutex(map_mutex);
	// Search the items on the specified tile for a specific layer
	if (entry_get_group_list(MAP_TABLE, map.c_str(), &eventlist, layer_name,
	MAP_ENTRY_EVENT_LIST, nullptr) == true)
	{
		while (eventlist[i] != nullptr)
		{
			if (entry_read_int(MAP_TABLE, map.c_str(), &mapx, layer_name,
			MAP_ENTRY_EVENT_LIST, eventlist[i], MAP_EVENT_TILE_X, nullptr) == false)
			{
				i++;
				continue;
			}

			if (entry_read_int(MAP_TABLE, map.c_str(), &mapy, layer_name,
			MAP_ENTRY_EVENT_LIST, eventlist[i], MAP_EVENT_TILE_Y, nullptr) == false)
			{
				i++;
				continue;
			}

			if (x == mapx && y == mapy)
			{
				event_id_num++;
				event_id = (char**) realloc(event_id, sizeof(char*) * (event_id_num + 1));
				event_id[event_id_num - 1] = strdup(eventlist[i]);
				event_id[event_id_num] = nullptr;
			}

			i++;
		}
	}
	deep_free(eventlist);
	// Search the items on the specified tile for all layers
	if (entry_get_group_list(MAP_TABLE, map.c_str(), &eventlist, MAP_ENTRY_EVENT_LIST, nullptr) == true)
	{
		while (eventlist[i] != nullptr)
		{
			if (entry_read_int(MAP_TABLE, map.c_str(), &mapx, MAP_ENTRY_EVENT_LIST, eventlist[i], MAP_EVENT_TILE_X, nullptr) == false)
			{
				i++;
				continue;
			}

			if (entry_read_int(MAP_TABLE, map.c_str(), &mapy, MAP_ENTRY_EVENT_LIST, eventlist[i], MAP_EVENT_TILE_Y, nullptr) == false)
			{
				i++;
				continue;
			}

			if (x == mapx && y == mapy)
			{
				event_id_num++;
				event_id = (char**) realloc(event_id, sizeof(char*) * (event_id_num + 1));
				event_id[event_id_num - 1] = strdup(eventlist[i]);
				event_id[event_id_num] = nullptr;
			}

			i++;
		}
	}
	deep_free(eventlist);

	SDL_UnlockMutex(map_mutex);

	return event_id;
}

/******************************************
 Add an event on map at given coordinate
 return nullptr if fails
 return the event id is success
 the return event id must be freed by caller
 ***********************************************/
char * map_add_event(const char * map, int layer, const char * script, int x, int y)
{
	char * id;
	char layer_name[SMALL_BUF];

	if (x < 0 || y < 0)
	{
		return nullptr;
	}

	sprintf(layer_name, "%s%d", MAP_KEY_LAYER, layer);

	// Make sure the MAP_ENTRY_EVENT_LIST group exists
	entry_group_create(MAP_TABLE, map, layer_name, MAP_ENTRY_EVENT_LIST, nullptr);

	id = entry_get_unused_group(MAP_TABLE, map, layer_name,
	MAP_ENTRY_EVENT_LIST, nullptr);
	if (id == nullptr)
	{
		return nullptr;
	}

	SDL_LockMutex(map_mutex);

	if (entry_write_int(MAP_TABLE, map, x, layer_name, MAP_ENTRY_EVENT_LIST, id,
	MAP_EVENT_TILE_X, nullptr) == false)
	{
		entry_remove_group(MAP_TABLE, map, id, layer_name, MAP_ENTRY_EVENT_LIST, nullptr);
		free(id);
		SDL_UnlockMutex(map_mutex);
		return nullptr;
	}
	if (entry_write_int(MAP_TABLE, map, y, layer_name, MAP_ENTRY_EVENT_LIST, id,
	MAP_EVENT_TILE_Y, nullptr) == false)
	{
		entry_remove_group(MAP_TABLE, map, id, layer_name, MAP_ENTRY_EVENT_LIST, nullptr);
		free(id);
		SDL_UnlockMutex(map_mutex);
		return nullptr;
	}

	if (entry_write_string(MAP_TABLE, map, script, layer_name,
	MAP_ENTRY_EVENT_LIST, id, MAP_EVENT_SCRIPT, nullptr) == false)
	{
		entry_remove_group(MAP_TABLE, map, id, layer_name, MAP_ENTRY_EVENT_LIST, nullptr);
		free(id);
		SDL_UnlockMutex(map_mutex);
		return nullptr;
	}

	SDL_UnlockMutex(map_mutex);

	// Send network notifications
	context_broadcast_map(map);

	return id;
}

/******************************************
 Add a parameter to the given event
 return false on error
 ***********************************************/
int map_add_event_param(const char * map, int layer, const char * event_id, const char * param)
{
	char layer_name[SMALL_BUF];

	sprintf(layer_name, "%s%d", MAP_KEY_LAYER, layer);

	// Make sure the param list exists
	entry_list_create(MAP_TABLE, map, layer_name, MAP_ENTRY_EVENT_LIST, event_id, MAP_EVENT_PARAM, nullptr);

	return entry_add_to_list(MAP_TABLE, map, param, layer_name,
	MAP_ENTRY_EVENT_LIST, event_id, MAP_EVENT_PARAM, nullptr);
}

/**********************************************
 Delete an event on map at given coordinate
 return false if fails
 **********************************************/
int map_delete_event(const char * map, int layer, const char * script, int x, int y)
{
	char ** eventlist;
	int i = 0;
	int mapx;
	int mapy;
	char * map_script = nullptr;
	const char * id = nullptr;
	char layer_name[SMALL_BUF];

	if (x < 0 || y < 0)
	{
		return false;
	}

	sprintf(layer_name, "%s%d", MAP_KEY_LAYER, layer);

	/* Manage concurrent acces to map files */
	SDL_LockMutex(map_mutex);
	/* Search events on the specified tile */
	if (entry_get_group_list(MAP_TABLE, map, &eventlist, layer_name,
	MAP_ENTRY_EVENT_LIST, nullptr) == false)
	{
		SDL_UnlockMutex(map_mutex);
		return false;
	}

	while (eventlist[i] != nullptr)
	{
		if (entry_read_int(MAP_TABLE, map, &mapx, layer_name,
		MAP_ENTRY_EVENT_LIST, eventlist[i], MAP_EVENT_TILE_X, nullptr) == false)
		{
			i++;
			continue;
		}
		if (entry_read_int(MAP_TABLE, map, &mapy, layer_name,
		MAP_ENTRY_EVENT_LIST, eventlist[i], MAP_EVENT_TILE_Y, nullptr) == false)
		{
			i++;
			continue;
		}
		if (entry_read_string(MAP_TABLE, map, &map_script, layer_name,
		MAP_ENTRY_EVENT_LIST, eventlist[i], MAP_EVENT_SCRIPT, nullptr) == false)
		{
			i++;
			continue;
		}
		if (x == mapx && y == mapy && !strcmp(map_script, script))
		{
			id = eventlist[i];
			free(map_script);
			break;
		}
		free(map_script);
		i++;
	}

	if (id == nullptr)
	{
		SDL_UnlockMutex(map_mutex);
		return false;
	}

	/* remove the event from the events list of the map */
	if (entry_remove_group(MAP_TABLE, map, id, layer_name, MAP_ENTRY_EVENT_LIST, nullptr) == false)
	{
		SDL_UnlockMutex(map_mutex);
		return false;
	}

	deep_free(eventlist);

	SDL_UnlockMutex(map_mutex);

	/* Send network notifications */
	context_broadcast_map(map);

	return true;
}

/************************************************
 return an array of character id on given map at x,y
 This array AND its content MUST be freed by caller
 ************************************************/
char ** map_get_character(const std::string & map, int x, int y)
{
	char ** character_list = nullptr;
	int character_num = 0;
	Context * ctx = context_get_first();

	if (x < 0 || y < 0)
	{
		return nullptr;
	}

	context_lock_list();
	while (ctx != nullptr)
	{
		if (ctx->getMap() == "")
		{
			ctx = ctx->m_next;
			continue;
		}
		if ((ctx->getTileX() == x) && (ctx->getTileY() == y) && (ctx->getMap() == map))
		{
			character_num++;
			character_list = (char**) realloc(character_list, sizeof(char*) * (character_num + 1));
			character_list[character_num - 1] = strdup(ctx->getId().c_str());
			character_list[character_num] = nullptr;
		}

		ctx = ctx->m_next;
	}
	context_unlock_list();

	return character_list;
}

/************************************************
 return an array of item id on given map at x,y
 This array AND its content MUST be freed by caller
 ************************************************/
char ** map_get_item(const char * map, int layer, int map_x, int map_y)
{
	char ** item_id = nullptr;
	char ** item_list = nullptr;
	int item_num = 0;
	int x;
	int y;
	int i;
	char layer_name[SMALL_BUF];

	if (map_x < 0 || map_y < 0)
	{
		return nullptr;
	}

	sprintf(layer_name, "%s%d", MAP_KEY_LAYER, layer);

	if (entry_get_group_list(MAP_TABLE, map, &item_id, layer_name,
	MAP_ENTRY_ITEM_LIST, nullptr) == false)
	{
		return nullptr;
	}

	i = 0;
	while (item_id[i] != nullptr)
	{
		if (entry_read_int(MAP_TABLE, map, &x, layer_name, MAP_ENTRY_ITEM_LIST, item_id[i], MAP_ITEM_TILE_X, nullptr) == false)
		{
			i++;
			continue;
		}

		if (entry_read_int(MAP_TABLE, map, &y, layer_name, MAP_ENTRY_ITEM_LIST, item_id[i], MAP_ITEM_TILE_Y, nullptr) == false)
		{
			i++;
			continue;
		}

		if (x == map_x && y == map_y)
		{
			item_num++;
			item_list = (char**) realloc(item_list, sizeof(char*) * (item_num + 1));
			item_list[item_num - 1] = strdup(item_id[i]);
			item_list[item_num] = nullptr;
		}
		i++;
	}

	deep_free(item_id);

	return item_list;
}

/************************************************
 Fill tx and ty with pixels coordinate of scpecified tile
 tx and/or ty may be nullptr
 tx and ty are not modified on error
 return false on error
 ************************************************/
int map_get_tile_coord(const char * map, int layer, int x, int y, int * tx, int * ty)
{
	layer_t * default_layer;

	if (map == nullptr)
	{
		return false;
	}

	if ((default_layer = map_layer_new(map, DEFAULT_LAYER, nullptr)) == nullptr)
	{
		return false;
	}

	if (tx)
	{
		*tx = map_t2p_x(x, y, default_layer);
	}
	if (ty)
	{
		*ty = map_t2p_y(x, y, default_layer);
	}

	map_layer_delete(default_layer);

	return true;
}

/******************************************
 Add a scenery on map at given coordinate
 return nullptr if fails
 return the scenery id is success
 the return scenery id must be freed by caller
 ***********************************************/
char * map_add_scenery(const char * map, int layer, int x, int y, const char * image_name)
{
	char layer_name[SMALL_BUF];
	char * id;

	if (x < 0 || y < 0)
	{
		return nullptr;
	}

	sprintf(layer_name, "%s%d", MAP_KEY_LAYER, layer);

	/* Make sure the MAP_KEY_SCENERY group exists */
	entry_group_create(MAP_TABLE, map, layer_name, MAP_KEY_SCENERY, nullptr);

	id = entry_get_unused_group(MAP_TABLE, map, layer_name, MAP_KEY_SCENERY, nullptr);
	if (id == nullptr)
	{
		return nullptr;
	}

	SDL_LockMutex(map_mutex);

	if (entry_write_int(MAP_TABLE, map, x, layer_name, MAP_KEY_SCENERY, id,
	MAP_KEY_SCENERY_X, nullptr) == false)
	{
		entry_remove_group(MAP_TABLE, map, id, layer_name, MAP_KEY_SCENERY, nullptr);
		SDL_UnlockMutex(map_mutex);
		free(id);
		return nullptr;
	}
	if (entry_write_int(MAP_TABLE, map, y, layer_name, MAP_KEY_SCENERY, id,
	MAP_KEY_SCENERY_Y, nullptr) == false)
	{
		entry_remove_group(MAP_TABLE, map, id, layer_name, MAP_KEY_SCENERY, nullptr);
		SDL_UnlockMutex(map_mutex);
		free(id);
		return nullptr;
	}
	if (entry_write_string(MAP_TABLE, map, image_name, layer_name,
	MAP_KEY_SCENERY, id, MAP_KEY_SCENERY_IMAGE, nullptr) == false)
	{
		entry_remove_group(MAP_TABLE, map, id, layer_name, MAP_KEY_SCENERY, nullptr);
		SDL_UnlockMutex(map_mutex);
		free(id);
		return nullptr;
	}

	SDL_UnlockMutex(map_mutex);

	// Send network notifications
	context_broadcast_map(map);

	return id;
}

/******************************************
 Add a layer filled with image_name on map
 return false on failure
 ***********************************************/
int map_add_layer(const char * map_name, int layer, int w, int h, int tile_w, int tile_h, const char * default_tile, const char * default_type)
{
	char layer_name[SMALL_BUF];

	sprintf(layer_name, "%s%d", MAP_KEY_LAYER, layer);

	if (w > 0)
	{
		if (entry_write_int(MAP_TABLE, map_name, w, layer_name, MAP_KEY_WIDTH, nullptr) == false)
		{
			return false;
		}
	}
	else
	{ // No specific dimension, take the map's global one
		if (entry_read_int(MAP_TABLE, map_name, &w, MAP_KEY_WIDTH, nullptr) == false)
		{
			return false;
		}
	}

	if (h > 0)
	{
		if (entry_write_int(MAP_TABLE, map_name, h, layer_name, MAP_KEY_HEIGHT, nullptr) == false)
		{
			return false;
		}
	}
	else
	{ // No specific dimension, take the map's global one
		if (entry_read_int(MAP_TABLE, map_name, &h, MAP_KEY_HEIGHT, nullptr) == false)
		{
			return false;
		}
	}

	if (tile_w > 0)
	{
		if (entry_write_int(MAP_TABLE, map_name, tile_w, layer_name,
		MAP_KEY_TILE_WIDTH, nullptr) == false)
		{
			return false;
		}
	}
	if (tile_h > 0)
	{
		if (entry_write_int(MAP_TABLE, map_name, tile_h, layer_name,
		MAP_KEY_TILE_HEIGHT, nullptr) == false)
		{
			return false;
		}
	}

	int i;
	char ** tile_array = (char**) malloc(((w * h) + 1) * sizeof(char *));

	// Write default tile
	for (i = 0; i < (w * h); i++)
	{
		tile_array[i] = (char *) default_tile;
	}
	tile_array[i] = nullptr; // End of list

	if (entry_write_list(MAP_TABLE, map_name, tile_array, layer_name,
	MAP_KEY_SET, nullptr) == false)
	{
		free(tile_array);
		return false;
	}

	// Write default type
	for (i = 0; i < (w * h); i++)
	{
		tile_array[i] = (char *) default_type;
	}
	tile_array[i] = nullptr; // End of list

	if (entry_write_list(MAP_TABLE, map_name, tile_array, layer_name,
	MAP_KEY_TYPE, nullptr) == false)
	{
		free(tile_array);
		return false;
	}

	free(tile_array);
	return true;
}

/******************************************
 remove a layer
 return false on failure
 ***********************************************/
int map_delete_layer(const char * map_name, int layer)
{
	char layer_name[SMALL_BUF];

	sprintf(layer_name, "%s%d", MAP_KEY_LAYER, layer);

	if (entry_remove_group(MAP_TABLE, map_name, layer_name, nullptr) == false)
	{
		return false;
	}

	return true;
}
