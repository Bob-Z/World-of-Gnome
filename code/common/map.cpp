/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2019 carabobz@gmail.com

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

#include "const.h"
#include "entry.h"
#include "map.h"
#include "mutex.h"
#include "syntax.h"
#include <stdlib.h>
#include <cstdio>

/************************************************
 create a layer_t struct to be used by t2p_x and t2p_y
 if layer_index == DEFAULT_LAYER or default_layer is nullptr, default value are filled in filled_layer
 return nullptr on error
 return a layer_t struct which needs to be freed by a call to map_layer_delete
 ************************************************/
layer_t * map_layer_new(const std::string & map, int layer_index, layer_t * default_layer)
{
	char layer_name[SMALL_BUF];
	char keyword[SMALL_BUF];
	int tiling_index = 0;
	char * zoom_str;
	int more;
	layer_t * layer = nullptr;

	layer = (layer_t*) malloc(sizeof(layer_t));

	layer->active = false;

	if (layer_index != DEFAULT_LAYER && default_layer != nullptr)
	{
		sprintf(layer_name, "%s%d", MAP_KEY_LAYER, layer_index);
		if (entry_exist(MAP_TABLE, map.c_str(), layer_name, MAP_KEY_SET, nullptr) == false)
		{
			free(layer);
			return nullptr;
		}

		// Automatic tiling
		layer->col_width[0] = default_layer->col_width[0];
		layer->col_height[0] = default_layer->col_height[0];
		layer->row_width[0] = default_layer->row_width[0];
		layer->row_height[0] = default_layer->row_height[0];

		layer->map_w = default_layer->map_w;
		entry_read_int(MAP_TABLE, map.c_str(), &layer->map_w, layer_name, MAP_KEY_WIDTH, nullptr);

		layer->map_h = default_layer->map_h;
		entry_read_int(MAP_TABLE, map.c_str(), &layer->map_h, layer_name, MAP_KEY_HEIGHT, nullptr);

		layer->tile_width = default_layer->tile_width;
		if (entry_read_int(MAP_TABLE, map.c_str(), &layer->tile_width, layer_name, MAP_KEY_TILE_WIDTH, nullptr) == true)
		{
			layer->col_width[0] = layer->tile_width;
		}

		layer->tile_height = default_layer->tile_height;
		if (entry_read_int(MAP_TABLE, map.c_str(), &layer->tile_height, layer_name, MAP_KEY_TILE_HEIGHT, nullptr) == true)
		{
			layer->row_height[0] = layer->tile_height;
		}

		layer->map_zoom = default_layer->map_zoom;
		if (entry_read_string(MAP_TABLE, map.c_str(), &zoom_str, layer_name, MAP_KEY_SPRITE_ZOOM, nullptr) == true)
		{
			layer->map_zoom = atof(zoom_str);
			free(zoom_str);
		}

		layer->row_num = default_layer->row_num;
		layer->col_num = default_layer->col_num;
	}
	else
	{
		if (entry_read_int(MAP_TABLE, map.c_str(), &layer->map_w, MAP_KEY_WIDTH, nullptr) == false)
		{
			free(layer);
			return nullptr;
		}
		if (entry_read_int(MAP_TABLE, map.c_str(), &layer->map_h, MAP_KEY_HEIGHT, nullptr) == false)
		{
			free(layer);
			return nullptr;
		}
		if (entry_read_int(MAP_TABLE, map.c_str(), &layer->tile_width, MAP_KEY_TILE_WIDTH, nullptr) == false)
		{
			free(layer);
			return nullptr;
		}
		if (entry_read_int(MAP_TABLE, map.c_str(), &layer->tile_height, MAP_KEY_TILE_HEIGHT, nullptr) == false)
		{
			free(layer);
			return nullptr;
		}

		// Automatic tiling
		layer->col_width[0] = layer->tile_width;
		layer->col_height[0] = 0;
		layer->row_width[0] = 0;
		layer->row_height[0] = layer->tile_height;

		layer->map_zoom = 1.0;
		if (entry_read_string(MAP_TABLE, map.c_str(), &zoom_str, MAP_KEY_SPRITE_ZOOM, nullptr) == true)
		{
			layer->map_zoom = atof(zoom_str);
			free(zoom_str);
		}

		layer->row_num = 1;
		layer->col_num = 1;
	}

	// Custom tiling
	for (tiling_index = 0; tiling_index < MAX_COL; tiling_index++)
	{
		more = false;

		if (layer_index != DEFAULT_LAYER && default_layer != nullptr)
		{
			if (tiling_index > 0)
			{
				layer->col_width[tiling_index] = default_layer->col_width[tiling_index];
				layer->col_height[tiling_index] = default_layer->col_height[tiling_index];
			}

			sprintf(keyword, "%s%d", MAP_KEY_COL_WIDTH, tiling_index);
			if (entry_read_int(MAP_TABLE, map.c_str(), &layer->col_width[tiling_index], layer_name, keyword, nullptr) == true)
			{
				more = true;
			}
			sprintf(keyword, "%s%d", MAP_KEY_COL_HEIGHT, tiling_index);
			if (entry_read_int(MAP_TABLE, map.c_str(), &layer->col_height[tiling_index], layer_name, keyword, nullptr) == true)
			{
				more = true;
			}
		}
		else
		{
			if (tiling_index > 0)
			{
				layer->col_width[tiling_index] = 0;
				layer->col_height[tiling_index] = 0;
			}

			sprintf(keyword, "%s%d", MAP_KEY_COL_WIDTH, tiling_index);
			if (entry_read_int(MAP_TABLE, map.c_str(), &layer->col_width[tiling_index], keyword, nullptr) == true)
			{
				more = true;
			}
			sprintf(keyword, "%s%d", MAP_KEY_COL_HEIGHT, tiling_index);
			if (entry_read_int(MAP_TABLE, map.c_str(), &layer->col_height[tiling_index], keyword, nullptr) == true)
			{
				more = true;
			}
		}
		if (more)
		{
			if (tiling_index > 0)
			{
				layer->col_num++;
			}
		}
	}

	for (tiling_index = 0; tiling_index < MAX_ROW; tiling_index++)
	{
		more = false;

		if (layer_index != DEFAULT_LAYER && default_layer != nullptr)
		{
			if (tiling_index > 0)
			{
				layer->row_width[tiling_index] = default_layer->row_width[tiling_index];
				layer->row_height[tiling_index] = default_layer->row_height[tiling_index];
			}

			sprintf(keyword, "%s%d", MAP_KEY_ROW_WIDTH, tiling_index);
			if (entry_read_int(MAP_TABLE, map.c_str(), &layer->row_width[tiling_index], layer_name, keyword, nullptr) == true)
			{
				more = true;
			}
			sprintf(keyword, "%s%d", MAP_KEY_ROW_HEIGHT, tiling_index);
			if (entry_read_int(MAP_TABLE, map.c_str(), &layer->row_height[tiling_index], layer_name, keyword, nullptr) == true)
			{
				more = true;
			}
		}
		else
		{
			if (tiling_index > 0)
			{
				layer->row_width[tiling_index] = 0;
				layer->row_height[tiling_index] = 0;
			}

			sprintf(keyword, "%s%d", MAP_KEY_ROW_WIDTH, tiling_index);
			if (entry_read_int(MAP_TABLE, map.c_str(), &layer->row_width[tiling_index], keyword, nullptr) == true)
			{
				more = true;
			}
			sprintf(keyword, "%s%d", MAP_KEY_ROW_HEIGHT, tiling_index);
			if (entry_read_int(MAP_TABLE, map.c_str(), &layer->row_height[tiling_index], keyword, nullptr) == true)
			{
				more = true;
			}
		}
		if (more)
		{
			if (tiling_index > 0)
			{
				layer->row_num++;
			}
		}
	}

	for (tiling_index = 0, layer->col_width_total = 0; tiling_index < layer->col_num; tiling_index++)
	{
		layer->col_width_total += layer->col_width[tiling_index];
	}
	for (tiling_index = 0, layer->col_height_total = 0; tiling_index < layer->col_num; tiling_index++)
	{
		layer->col_height_total += layer->col_height[tiling_index];
	}
	for (tiling_index = 0, layer->row_width_total = 0; tiling_index < layer->row_num; tiling_index++)
	{
		layer->row_width_total += layer->row_width[tiling_index];
	}
	for (tiling_index = 0, layer->row_height_total = 0; tiling_index < layer->row_num; tiling_index++)
	{
		layer->row_height_total += layer->row_height[tiling_index];
	}

	layer->active = true;

	return layer;
}

/************************************************
 Delete a layer_t struct created by map_layer_new
 ************************************************/
void map_layer_delete(layer_t * layer)
{
	free(layer);
}

/**********************************
 Convert tiles coordinates into pixels coordinates
 **********************************/
int map_t2p_x(int x, int y, layer_t * layer)
{
	int i;
	int res;

	res = (x / layer->col_num) * layer->col_width_total;

	for (i = 0; i < x % layer->col_num; i++)
	{
		res += layer->col_width[i];
	}

	res += (y / layer->row_num) * layer->row_width_total;

	for (i = 0; i < y % layer->row_num; i++)
	{
		res += layer->row_width[i];
	}

	return res;
}

/**********************************
 Convert tiles coordinates into pixels coordinates
 **********************************/
int map_t2p_y(int x, int y, layer_t * layer)
{
	int i;
	int res;

	res = (x / layer->col_num) * layer->col_height_total;

	for (i = 0; i < x % layer->col_num; i++)
	{
		res += layer->col_height[i];
	}

	res += (y / layer->row_num) * layer->row_height_total;

	for (i = 0; i < y % layer->row_num; i++)
	{
		res += layer->row_height[i];
	}

	return res;
}

