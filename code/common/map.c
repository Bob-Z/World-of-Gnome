/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013 carabobz@gmail.com

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

#include <glib.h>
#include <gio/gio.h>
#include "common.h"

/* avoid 2 server's thread to change a map file at the same time */
static GStaticMutex map_mutex = G_STATIC_MUTEX_INIT;

/***********************************
Create a new map.
Return the name of the new map
*************************************/
gchar * map_new(gint x,gint y, gint tile_x, gint tile_y, gchar * default_tile)
{
	gchar * map_name;
	gchar ** tile_array;
	gint i;

	map_name = file_new(MAP_TABLE);
	if(map_name == NULL) return NULL;

	
	if (!write_int(MAP_TABLE,map_name,x,MAP_KEY_SIZE_X, NULL) ) {
		g_free(map_name);
		return NULL;
	}
	if (!write_int(MAP_TABLE,map_name,y,MAP_KEY_SIZE_Y, NULL) ) {
		g_free(map_name);
		return NULL;
	}
	if (!write_int(MAP_TABLE,map_name,tile_x,MAP_KEY_TILE_SIZE_X, NULL) ) {
		g_free(map_name);
		return NULL;
	}
	if (!write_int(MAP_TABLE,map_name,tile_y,MAP_KEY_TILE_SIZE_Y, NULL) ) {
		g_free(map_name);
		return NULL;
	}

	tile_array=g_malloc0(((x*y)+1)*sizeof(gchar *));
	for(i=0;i<(x*y);i++) {
		tile_array[i] = default_tile;
	}
	if (!write_list(MAP_TABLE,map_name,tile_array,MAP_KEY_SET, NULL) ) {
		g_free(map_name);
		return NULL;
	}

	return map_name;
}

/***********************************
check if context is allowed to go on a tile
return TRUE if the context is allowed to go to the tile at coord x,y
*************************************/
gboolean map_check_tile(context_t * context,gchar * map, gint x,gint y)
{
        gchar ** map_tiles;
        gchar ** allowed_tile;
        const gchar * tile_type;
        gint i=0;
	gint size_x = 0;
	gint size_y = 0;

        if(!read_list(MAP_TABLE,map,&map_tiles,MAP_KEY_SET,NULL)) {
                return FALSE;
        }

        if(!read_int(MAP_TABLE,map,&size_x,MAP_KEY_SIZE_X,NULL)) {
                return FALSE;
        }

        if(!read_int(MAP_TABLE,map,&size_y,MAP_KEY_SIZE_Y,NULL)) {
                return FALSE;
        }

	/* sanity_check */
	if( x < 0 || y < 0 || x >= size_x || y >= size_y ) {
		return FALSE;
	}

        if(!read_string(TILE_TABLE,map_tiles[(size_x*y)+x],&tile_type,TILE_KEY_TYPE,NULL)) {
                g_free(map_tiles);
                /* no type for this tile, allowed for everyone */
                return TRUE;
        }

	/* try specific allowed tile */
	if(!read_list(CHARACTER_TABLE,context->id,&allowed_tile,AVATAR_KEY_ALLOWED_TILE,NULL)) {
		allowed_tile = NULL;
	}

	/* try general avatar allowed tile */
	if( allowed_tile == NULL ) {
		if(!read_list(AVATAR_TABLE,context->type,&allowed_tile,AVATAR_KEY_ALLOWED_TILE,NULL)) {
		/* no allowed_type -> this character is allowed on all tile*/
			g_free(map_tiles);
			return TRUE;
		}
	}

        while( allowed_tile[i] != NULL ) {
                if( g_strcmp0(allowed_tile[i], tile_type) == 0 ) {
                        g_free(allowed_tile);
                        g_free(map_tiles);
                        return TRUE;
                }
                i++;
        }

        g_free(allowed_tile);
        g_free(map_tiles);

        return FALSE;
}

/* delete an item on context's map */
gchar * map_delete_item(const gchar * map, gint x, gint y)
{
	gchar ** itemlist;
	gint i=0;
	gint mapx;
	gint mapy;
	const gchar * id = NULL;
	gchar * saved_item = NULL;

	/* Manage concurrent acces to map files */
	g_static_mutex_lock(&map_mutex);
	/* Search the items on the specified tile */
	if(!get_group_list(MAP_TABLE,map,&itemlist,MAP_ENTRY_ITEM_LIST,NULL)) {
		g_static_mutex_unlock(&map_mutex);
		return NULL;
 	}

	while(itemlist[i] != NULL) {
		if( !read_int(MAP_TABLE,map,&mapx,MAP_ENTRY_ITEM_LIST,itemlist[i],MAP_ITEM_POS_X,NULL) ) {
			g_static_mutex_unlock(&map_mutex);
			g_free(itemlist);
			return NULL;
 		}

		if( !read_int(MAP_TABLE,map,&mapy,MAP_ENTRY_ITEM_LIST,itemlist[i],MAP_ITEM_POS_Y,NULL) ) {
			g_static_mutex_unlock(&map_mutex);
			g_free(itemlist);
			return NULL;
 		}

		if( x == mapx && y == mapy ) {
			id = itemlist[i];
			saved_item = g_strdup(itemlist[i]);
			break;
		}

		i++;
	}

	if( id == NULL ) {
		g_free(itemlist);
		if( saved_item) {
			g_free(saved_item);
		}
		g_static_mutex_unlock(&map_mutex);
		return NULL;
	}

	/* remove the item from the item list of the map */
	if(!remove_group(MAP_TABLE,map,id,MAP_ENTRY_ITEM_LIST,NULL)) {
		g_free(itemlist);
		g_free(saved_item);
		g_static_mutex_unlock(&map_mutex);
		return NULL;
	}

	g_free(itemlist);

	g_static_mutex_unlock(&map_mutex);

	/* Send network notifications */
	context_broadcast_file(MAP_TABLE,map,TRUE);

	return saved_item;
}
/******************************************/
/* Add an item on map at given coordinate */
/* return -1 if fails                      */
/******************************************/
gint map_add_item(const gchar * map, const gchar * id, gint x, gint y)
{

	g_static_mutex_lock(&map_mutex);

	if (!write_int(MAP_TABLE,map,x,MAP_ENTRY_ITEM_LIST,id,MAP_ITEM_POS_X, NULL) ) {
		remove_group(MAP_TABLE,map,id,MAP_ENTRY_ITEM_LIST,NULL);
		g_static_mutex_unlock(&map_mutex);
		return -1;
	}
	if (!write_int(MAP_TABLE,map,y,MAP_ENTRY_ITEM_LIST,id,MAP_ITEM_POS_Y, NULL) ) {
		remove_group(MAP_TABLE,map,id,MAP_ENTRY_ITEM_LIST,NULL);
		g_static_mutex_unlock(&map_mutex);
		return -1;
	}

	g_static_mutex_unlock(&map_mutex);

	/* Send network notifications */
	context_broadcast_file(MAP_TABLE,map,TRUE);

	return 0;
}
/************************************/
/* Write a new tile into a map file */
/* return -1 if fails */
/************************************/
int map_set_tile(const gchar * map,const gchar * tile,gint x, gint y)
{
	/* Extract params */
	const gchar * value = NULL;
	int sizex = -1;
	gint index;
	
	/* Check parameters sanity */
	if(map == NULL || tile == NULL) {
		return -1;
	}

	if( x < 0 || y < 0) {
		return -1;
	}

	/* Manage concurrent acces to map files */
	g_static_mutex_lock(&map_mutex);

	/* read size of map */
	if(!read_int(MAP_TABLE,map,&sizex,MAP_KEY_SIZE_X,NULL)) {
		g_static_mutex_unlock(&map_mutex);
		return -1;
	}

	index = sizex * y + x;

	/* read map tile */
	if(!read_list_index(MAP_TABLE,map,&value, index,MAP_KEY_SET,NULL)) {
		g_static_mutex_unlock(&map_mutex);
                return -1;
        }

	/* Do not change the tile if it already the requested tile */
        if( g_strcmp0(value, tile) == 0 ) {
                g_static_mutex_unlock(&map_mutex);
                return 0;
        }

	if( write_list_index(MAP_TABLE, map, tile,index, MAP_KEY_SET,NULL ) ) {
		context_broadcast_file(MAP_TABLE,map,TRUE);
	}

	g_static_mutex_unlock(&map_mutex);

	return 0;
}

/*********************************************/
/* return the name of the tile on map at x,y */
/*********************************************/
gchar * map_get_tile(const gchar * map,gint x, gint y)
{
        gchar ** map_tiles;
	gint map_size_x;

        if(!read_list(MAP_TABLE,map,&map_tiles,MAP_KEY_SET,NULL)) {
                return NULL;
        }

	if(!read_int(MAP_TABLE,map,&map_size_x,MAP_KEY_SIZE_X,NULL)) {
                return NULL;
        }

	if( map_tiles[(map_size_x*y)+x] ) {
		return g_strdup(map_tiles[(map_size_x*y)+x]);
	}

	return NULL;
}

/***********************************************/
/* return an array of script name on map at x,y */
/* This array MUST be freed by caller  */
/***********************************************/
const gchar ** map_get_event(const gchar * map,gint x, gint y)
{
	gchar ** eventlist = NULL;
	gint i=0;
	gint mapx;
	gint mapy;
	const gchar * s = NULL;
	const gchar ** script = NULL;
	int script_num = 0;

	/* Manage concurrent acces to map files */
	g_static_mutex_lock(&map_mutex);
	/* Search the items on the specified tile */
	if(!get_group_list(MAP_TABLE,map,&eventlist,MAP_ENTRY_EVENT_LIST,NULL)) {
		g_static_mutex_unlock(&map_mutex);
		return NULL;
 	}

	while(eventlist[i] != NULL) {
		if( !read_int(MAP_TABLE,map,&mapx,MAP_ENTRY_EVENT_LIST,eventlist[i],MAP_EVENT_POS_X,NULL) ) {
			i++;
			continue;
 		}

		if( !read_int(MAP_TABLE,map,&mapy,MAP_ENTRY_EVENT_LIST,eventlist[i],MAP_EVENT_POS_Y,NULL) ) {
			i++;
			continue;
 		}

		if( x == mapx && y == mapy ) {
			if( read_string(MAP_TABLE,map,&s,MAP_ENTRY_EVENT_LIST,eventlist[i],MAP_EVENT_SCRIPT,NULL) ) {
				script_num++;
				script=g_realloc(script,sizeof(gchar*)*(script_num+1));
				script[script_num-1] = s;
				script[script_num] = NULL;
			}
		}

		i++;
	}
	g_static_mutex_unlock(&map_mutex);

	return script;
}

/******************************************
Add an event on map at given coordinate
 return NULL if fails
 return the event id is success
 the return event id must be freed by caller
***********************************************/
gchar * map_add_event(const gchar * map, const gchar * script, gint x, gint y )
{
	gchar * id;

	/* Make sure the MAP_ENTRY_EVENT_LIST group exist */
	group_create(MAP_TABLE,map,MAP_ENTRY_EVENT_LIST,NULL);

	id = get_unused_group(MAP_TABLE,map,MAP_ENTRY_EVENT_LIST,NULL);
	if(id == NULL) {
		return NULL;
	}

	g_static_mutex_lock(&map_mutex);

	if (!write_int(MAP_TABLE,map,x,MAP_ENTRY_EVENT_LIST,id,MAP_EVENT_POS_X, NULL) ) {
		remove_group(MAP_TABLE,map,id,MAP_ENTRY_EVENT_LIST,NULL);
		g_free(id);
		g_static_mutex_unlock(&map_mutex);
		return NULL;
	}
	if (!write_int(MAP_TABLE,map,y,MAP_ENTRY_EVENT_LIST,id,MAP_EVENT_POS_Y, NULL) ) {
		remove_group(MAP_TABLE,map,id,MAP_ENTRY_EVENT_LIST,NULL);
		g_free(id);
		g_static_mutex_unlock(&map_mutex);
		return NULL;
	}

	if (!write_string(MAP_TABLE,map,script,MAP_ENTRY_EVENT_LIST,id,MAP_EVENT_SCRIPT, NULL) ) {
		remove_group(MAP_TABLE,map,id,MAP_ENTRY_EVENT_LIST,NULL);
		g_free(id);
		g_static_mutex_unlock(&map_mutex);
		return NULL;
	}

	g_static_mutex_unlock(&map_mutex);

	/* Send network notifications */
	context_broadcast_file(MAP_TABLE,map,TRUE);

	return id;
}

/**********************************************/
/* Delete an event on map at given coordinate */
/* return -1 if fails                         */
/**********************************************/
gint map_delete_event(const gchar * map, const gchar * script, gint x, gint y)
{
	gchar ** eventlist;
	gint i=0;
	gint mapx;
	gint mapy;
	const gchar * s;
	const gchar * id = NULL;

	/* Manage concurrent acces to map files */
	g_static_mutex_lock(&map_mutex);
	/* Search events on the specified tile */
	if(!get_group_list(MAP_TABLE,map,&eventlist,MAP_ENTRY_EVENT_LIST,NULL)) {
		g_static_mutex_unlock(&map_mutex);
		return -1;
 	}

	while(eventlist[i] != NULL) {
		if( !read_int(MAP_TABLE,map,&mapx,MAP_ENTRY_EVENT_LIST,eventlist[i],MAP_EVENT_POS_X,NULL) ) {
			i++;
			continue;
 		}
		if( !read_int(MAP_TABLE,map,&mapy,MAP_ENTRY_EVENT_LIST,eventlist[i],MAP_EVENT_POS_Y,NULL) ) {
			i++;
			continue;
 		}
		if( !read_string(MAP_TABLE,map,&s,MAP_ENTRY_EVENT_LIST,eventlist[i],MAP_EVENT_SCRIPT,NULL) ) {
			i++;
			continue;
 		}
		if( x == mapx && y == mapy && !g_strcmp0(s,script) ) {
			id = eventlist[i];
			break;
		}
		i++;
	}

	g_free(eventlist);

	if( id == NULL ) {
		g_static_mutex_unlock(&map_mutex);
		return -1;
	}

	/* remove the event from the events list of the map */
	if(!remove_group(MAP_TABLE,map,id,MAP_ENTRY_EVENT_LIST,NULL)) {
		g_static_mutex_unlock(&map_mutex);
		return -1;
	}

	g_static_mutex_unlock(&map_mutex);

	/* Send network notifications */
	context_broadcast_file(MAP_TABLE,map,TRUE);

	return 0;
}
