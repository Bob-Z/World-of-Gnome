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

#include <glib.h>
#include <gio/gio.h>
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include "../server/action.h"

/***********************************
Create a new map.
Return the name of the new map
*************************************/
gchar * map_new(gint x,gint y, gint tile_x, gint tile_y, gchar * default_tile)
{
	gchar * map_name;
	gchar ** tile_array;
	gint i;

	if( x<0 || y<0 ) {
		return NULL;
	}

	map_name = file_new(MAP_TABLE);
	if(map_name == NULL) {
		return NULL;
	}


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
	for(i=0; i<(x*y); i++) {
		tile_array[i] = default_tile;
	}
	if (!write_list(MAP_TABLE,map_name,tile_array,MAP_KEY_SET, NULL) ) {
		g_free(map_name);
		return NULL;
	}

	return map_name;
}

/***********************************
check if id is allowed to go on a tile
return TRUE if the context is allowed to go to the tile at coord x,y
*************************************/
gboolean map_check_tile(context_t * ctx,char * id, const gchar * map, gint x,gint y)
{
	const gchar * action;
	char sx[64];
	char sy[64];
	char * param[5];
	gboolean res;
	gchar ** map_tiles;
	gchar ** allowed_tile;
	const gchar * tile_type;
	gint i=0;
	gint size_x = 0;
	gint size_y = 0;

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

	/* If there is a allowed_tile_script, run it */
	if(read_string(CHARACTER_TABLE,id,&action, CHARACTER_KEY_ALLOWED_TILE_SCRIPT, NULL)) {
		param[0] = id;
		param[1] = (char *)map;
		sprintf(sx,"%d",x);
		sprintf(sy,"%d",y);
		param[2] = sx;
		param[3] = sy;
		param[4] = NULL;
		res = action_execute_script(ctx,action,param);
		return res;
	}

	/* Read tile list on this map */
	if(!read_list(MAP_TABLE,map,&map_tiles,MAP_KEY_SET,NULL)) {
		return FALSE;
	}

	/* Check the tile has a type */
	if(!read_string(TILE_TABLE,map_tiles[(size_x*y)+x],&tile_type,TILE_KEY_TYPE,NULL)) {
		g_free(map_tiles);
		/* this tile has no type, allowed for everyone */
		return TRUE;
	}

	/* If there is allowed_tile list, check it */
	if(read_list(CHARACTER_TABLE,id,&allowed_tile,CHARACTER_KEY_ALLOWED_TILE,NULL)) {
		i=0;
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

	/* Allow all tiles by default */
	return TRUE;
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

	if( x<0 || y<0 ) {
		return NULL;
	}

	/* Manage concurrent acces to map files */
	SDL_LockMutex(map_mutex);
	/* Search the items on the specified tile */
	if(!get_group_list(MAP_TABLE,map,&itemlist,MAP_ENTRY_ITEM_LIST,NULL)) {
		SDL_UnlockMutex(map_mutex);
		return NULL;
	}

	while(itemlist[i] != NULL) {
		if( !read_int(MAP_TABLE,map,&mapx,MAP_ENTRY_ITEM_LIST,itemlist[i],MAP_ITEM_POS_X,NULL) ) {
			SDL_UnlockMutex(map_mutex);
			g_free(itemlist);
			return NULL;
		}

		if( !read_int(MAP_TABLE,map,&mapy,MAP_ENTRY_ITEM_LIST,itemlist[i],MAP_ITEM_POS_Y,NULL) ) {
			SDL_UnlockMutex(map_mutex);
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
		SDL_UnlockMutex(map_mutex);
		return NULL;
	}

	/* remove the item from the item list of the map */
	if(!remove_group(MAP_TABLE,map,id,MAP_ENTRY_ITEM_LIST,NULL)) {
		g_free(itemlist);
		g_free(saved_item);
		SDL_UnlockMutex(map_mutex);
		return NULL;
	}

	g_free(itemlist);

	SDL_UnlockMutex(map_mutex);

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

	if( x<0 || y<0 ) {
		return -1;
	}

	SDL_LockMutex(map_mutex);

	if (!write_int(MAP_TABLE,map,x,MAP_ENTRY_ITEM_LIST,id,MAP_ITEM_POS_X, NULL) ) {
		remove_group(MAP_TABLE,map,id,MAP_ENTRY_ITEM_LIST,NULL);
		SDL_UnlockMutex(map_mutex);
		return -1;
	}
	if (!write_int(MAP_TABLE,map,y,MAP_ENTRY_ITEM_LIST,id,MAP_ITEM_POS_Y, NULL) ) {
		remove_group(MAP_TABLE,map,id,MAP_ENTRY_ITEM_LIST,NULL);
		SDL_UnlockMutex(map_mutex);
		return -1;
	}

	SDL_UnlockMutex(map_mutex);

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
	SDL_LockMutex(map_mutex);

	/* read size of map */
	if(!read_int(MAP_TABLE,map,&sizex,MAP_KEY_SIZE_X,NULL)) {
		SDL_UnlockMutex(map_mutex);
		return -1;
	}

	index = sizex * y + x;

	/* read map tile */
	if(!read_list_index(MAP_TABLE,map,&value, index,MAP_KEY_SET,NULL)) {
		SDL_UnlockMutex(map_mutex);
		return -1;
	}

	/* Do not change the tile if it already the requested tile */
	if( g_strcmp0(value, tile) == 0 ) {
		SDL_UnlockMutex(map_mutex);
		return 0;
	}

	if( write_list_index(MAP_TABLE, map, tile,index, MAP_KEY_SET,NULL ) ) {
		context_broadcast_file(MAP_TABLE,map,TRUE);
	}

	SDL_UnlockMutex(map_mutex);

	return 0;
}

/********************************************
 return the name of the tile on map at x,y
 return must be freed by caller
********************************************/
gchar * map_get_tile(const gchar * map,gint x, gint y)
{
	gchar ** map_tiles;
	gint map_size_x;
	gint map_size_y;


	if(!read_list(MAP_TABLE,map,&map_tiles,MAP_KEY_SET,NULL)) {
		return NULL;
	}

	if(!read_int(MAP_TABLE,map,&map_size_x,MAP_KEY_SIZE_X,NULL)) {
		return NULL;
	}

	if(!read_int(MAP_TABLE,map,&map_size_y,MAP_KEY_SIZE_Y,NULL)) {
		return NULL;
	}

	if( x<0 || y<0 || x >= map_size_x || y >= map_size_y ) {
		return NULL;
	}

	if( map_tiles[(map_size_x*y)+x] ) {
		return g_strdup(map_tiles[(map_size_x*y)+x]);
	}

	return NULL;
}

/********************************************
 return the type of the tile on map at x,y
********************************************/
const gchar * map_get_tile_type(const gchar * map,gint x, gint y)
{
	gchar ** map_tiles;
	gint map_size_x;
	gchar * tile;
	const gchar * type;

	if( x<0 || y<0) {
		return NULL;
	}

	if(!read_list(MAP_TABLE,map,&map_tiles,MAP_KEY_SET,NULL)) {
		return NULL;
	}

	if(!read_int(MAP_TABLE,map,&map_size_x,MAP_KEY_SIZE_X,NULL)) {
		return NULL;
	}

	tile = map_tiles[(map_size_x*y)+x];
	if( tile ) {
		if(!read_string(TILE_TABLE,tile,&type,TILE_KEY_TYPE,NULL)) {
			return NULL;
		}

		return type;
	}

	return NULL;
}

/************************************************
 return an array of event id on given map at x,y
 This array MUST be freed by caller
************************************************/
gchar ** map_get_event(const gchar * map,gint x, gint y)
{
	gchar ** eventlist = NULL;
	gchar ** event_id = NULL;
	gint i=0;
	gint mapx;
	gint mapy;
	int event_id_num = 0;

	if( x<0 || y<0 ) {
		return NULL;
	}

	/* Manage concurrent acces to map files */
	SDL_LockMutex(map_mutex);
	/* Search the items on the specified tile */
	if(!get_group_list(MAP_TABLE,map,&eventlist,MAP_ENTRY_EVENT_LIST,NULL)) {
		SDL_UnlockMutex(map_mutex);
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
			event_id_num++;
			event_id=g_realloc(event_id,sizeof(gchar*)*(event_id_num+1));
			event_id[event_id_num-1] = eventlist[i];
			event_id[event_id_num] = NULL;
		}

		i++;
	}
	SDL_UnlockMutex(map_mutex);

	return event_id;
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

	if( x<0 || y<0 ) {
		return NULL;
	}

	/* Make sure the MAP_ENTRY_EVENT_LIST group exists */
	group_create(MAP_TABLE,map,MAP_ENTRY_EVENT_LIST,NULL);

	id = get_unused_group(MAP_TABLE,map,MAP_ENTRY_EVENT_LIST,NULL);
	if(id == NULL) {
		return NULL;
	}

	SDL_LockMutex(map_mutex);

	if (!write_int(MAP_TABLE,map,x,MAP_ENTRY_EVENT_LIST,id,MAP_EVENT_POS_X, NULL) ) {
		remove_group(MAP_TABLE,map,id,MAP_ENTRY_EVENT_LIST,NULL);
		g_free(id);
		SDL_UnlockMutex(map_mutex);
		return NULL;
	}
	if (!write_int(MAP_TABLE,map,y,MAP_ENTRY_EVENT_LIST,id,MAP_EVENT_POS_Y, NULL) ) {
		remove_group(MAP_TABLE,map,id,MAP_ENTRY_EVENT_LIST,NULL);
		g_free(id);
		SDL_UnlockMutex(map_mutex);
		return NULL;
	}

	if (!write_string(MAP_TABLE,map,script,MAP_ENTRY_EVENT_LIST,id,MAP_EVENT_SCRIPT, NULL) ) {
		remove_group(MAP_TABLE,map,id,MAP_ENTRY_EVENT_LIST,NULL);
		g_free(id);
		SDL_UnlockMutex(map_mutex);
		return NULL;
	}

	SDL_UnlockMutex(map_mutex);

	/* Send network notifications */
	context_broadcast_file(MAP_TABLE,map,TRUE);

	return id;
}

/******************************************
 Add a parameter to the given event
 return 0 if fails
***********************************************/
gboolean map_add_event_param(const gchar * map, const gchar * event_id, const gchar * param)
{
	/* Make sure the param list exists */
	list_create(MAP_TABLE,map,MAP_ENTRY_EVENT_LIST,event_id,MAP_EVENT_PARAM,NULL);

	return add_to_list(MAP_TABLE,map,param,MAP_ENTRY_EVENT_LIST,event_id,MAP_EVENT_PARAM,NULL);
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

	if( x<0 || y<0 ) {
		return -1;
	}

	/* Manage concurrent acces to map files */
	SDL_LockMutex(map_mutex);
	/* Search events on the specified tile */
	if(!get_group_list(MAP_TABLE,map,&eventlist,MAP_ENTRY_EVENT_LIST,NULL)) {
		SDL_UnlockMutex(map_mutex);
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
		SDL_UnlockMutex(map_mutex);
		return -1;
	}

	/* remove the event from the events list of the map */
	if(!remove_group(MAP_TABLE,map,id,MAP_ENTRY_EVENT_LIST,NULL)) {
		SDL_UnlockMutex(map_mutex);
		return -1;
	}

	SDL_UnlockMutex(map_mutex);

	/* Send network notifications */
	context_broadcast_file(MAP_TABLE,map,TRUE);

	return 0;
}

/************************************************
 return an array of character id on given map at x,y
 This array AND its content MUST be freed by caller
************************************************/
char ** map_get_character(const gchar * map,gint x, gint y)
{
	char ** character_list = NULL;
	int character_num = 0;
	context_t * ctx = context_get_list_first();

	if( x<0 || y<0 ) {
		return NULL;
	}

	context_lock_list();
	while(ctx!= NULL) {
		if(ctx->map == NULL) {
			ctx = ctx->next;
			continue;
		}
		if(ctx->pos_x == x && ctx->pos_y == y && !strcmp(ctx->map,map)){
			character_num++;
			character_list=realloc(character_list,sizeof(char*)*(character_num+1));
			character_list[character_num-1] = strdup(ctx->id);
			character_list[character_num]=NULL;
		}

		ctx = ctx->next;
	}
	context_unlock_list();

	return character_list;
}
