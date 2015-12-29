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
#include <stdlib.h>
#include <string.h>
#include "action.h"

/***********************************
return string MUST BE FREED
***********************************/
static char * get_tile_type_through_layer(const char * map, int layer, int x, int y)
{
        char * type;
        while( layer >= 0 ) {
                type = map_get_tile_type(map,layer,x,y);
                if( type ) {
                        return type;
                }
                layer--;
        }

        return NULL;
}

/***********************************
check if id is allowed to go on a tile
return 1 if the context is allowed to go to the tile at coord x,y
return 0 if the context is NOT allowed to go to the tile at coord x,y
return RET_FAIL on error or no data found
*************************************/
int map_check_tile(context_t * ctx,char * id, const char * map, int layer, int x,int y)
{
	char * script;
	char sx[64];
	char sy[64];
	char * param[5];
	int res;
	char * tile_type;
	char ** allowed_tile;
	int i=0;
	int width = 0;
	int height = 0;

	if(!entry_read_int(MAP_TABLE,map,&width,MAP_KEY_WIDTH,NULL)) {
		return RET_FAIL;
	}
	if(!entry_read_int(MAP_TABLE,map,&height,MAP_KEY_HEIGHT,NULL)) {
		return RET_FAIL;
	}

	if( x < 0 || y < 0 || x >= width || y >= height ) {
		return 0;
	}

	/* If there is a allowed_tile_script, run it */
	if(entry_read_string(CHARACTER_TABLE,id,&script, CHARACTER_KEY_ALLOWED_TILE_SCRIPT, NULL)) {
		param[0] = id;
		param[1] = (char *)map;
		sprintf(sx,"%d",x);
		sprintf(sy,"%d",y);
		param[2] = sx;
		param[3] = sy;
		param[4] = NULL;
		res = action_execute_script(ctx,script,param);
		free(script);
		return res;
	}

	/* Read tile at given index on this map */
	entry_read_int(CHARACTER_TABLE,id,&layer,CHARACTER_LAYER,NULL);
	tile_type = get_tile_type_through_layer(map, layer, x, y);

	/* Allow tile if no type defined */
	if( tile_type == NULL ) {
		return 1;
	}

	/* Allow tile if its type is empty (i.e. "") */
	if( tile_type[0] == 0 ) {
		free(tile_type);
		return 1;
	}

	/* If there is allowed_tile list, check it */
	if(entry_read_list(CHARACTER_TABLE,id,&allowed_tile,CHARACTER_KEY_ALLOWED_TILE,NULL)) {
		i=0;
		while( allowed_tile[i] != NULL ) {
			if( strcmp(allowed_tile[i], tile_type) == 0 ) {
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
	/* Allow all tiles by default */
	return 1;
}

