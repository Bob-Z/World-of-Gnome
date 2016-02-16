/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013-2016 carabobz@gmail.com

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
#include "map_server.h"

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

/***********************************
Create a new map or add a map layer.
Return the name of the new map
*************************************/
char * map_new(const char *suggested_name,int layer, int w,int h, int tile_w, int tile_h, const char * default_tile, const char * default_type)
{
	char * map_name;
	char ** tile_array;
	int i;
	char layer_name[SMALL_BUF];

	if( w<0 || h<0 ) {
		return NULL;
	}

	map_name = file_new(MAP_TABLE,suggested_name);
	/* Map creation may fail because file already exists. Try the suggested name instead. */
	if( map_name == NULL ) {
		map_name = strdup(suggested_name);
	}

	if (!entry_write_int(MAP_TABLE,map_name,w,MAP_KEY_HEIGHT,NULL) ) {
		free(map_name);
		return NULL;
	}
	if (!entry_write_int(MAP_TABLE,map_name,h,MAP_KEY_WIDTH,NULL) ) {
		free(map_name);
		return NULL;
	}
	if (!entry_write_int(MAP_TABLE,map_name,tile_w,MAP_KEY_TILE_WIDTH,NULL) ) {
		free(map_name);
		return NULL;
	}
	if (!entry_write_int(MAP_TABLE,map_name,tile_h,MAP_KEY_TILE_HEIGHT,NULL) ) {
		free(map_name);
		return NULL;
	}

	tile_array=malloc(((w*h)+1)*sizeof(char *));

	sprintf(layer_name,"%s%d",MAP_KEY_LAYER,layer);

	/* Write default tile */
	for(i=0; i<(w*h); i++) {
		tile_array[i] = (char *)default_tile;
	}
	tile_array[i] = NULL; /* End of list */

	if (!entry_write_list(MAP_TABLE,map_name,tile_array,layer_name,MAP_KEY_SET, NULL) ) {
		free(map_name);
		return NULL;
	}

	/* Write default type */
	for(i=0; i<(w*h); i++) {
		tile_array[i] = (char *)default_type;
	}
	tile_array[i] = NULL; /* End of list */

	if (!entry_write_list(MAP_TABLE,map_name,tile_array,layer_name,MAP_KEY_TYPE, NULL) ) {
		free(map_name);
		return NULL;
	}

	return map_name;
}

/**************************************
delete an item on context's map
**************************************/
char * map_delete_item(const char * map, int layer, int x, int y)
{
	char ** itemlist;
	int i=0;
	int mapx;
	int mapy;
	const char * id = NULL;
	char * saved_item = NULL;
	char layer_name[SMALL_BUF];

	if( x<0 || y<0 ) {
		return NULL;
	}

	sprintf(layer_name,"%s%d",MAP_KEY_LAYER,layer);
	/* Manage concurrent acces to map files */
	SDL_LockMutex(map_mutex);
	/* Search the items on the specified tile */
	if(!entry_get_group_list(MAP_TABLE,map,&itemlist,layer_name,MAP_ENTRY_ITEM_LIST,NULL)) {
		SDL_UnlockMutex(map_mutex);
		return NULL;
	}

	while(itemlist[i] != NULL) {
		if( !entry_read_int(MAP_TABLE,map,&mapx,layer_name,MAP_ENTRY_ITEM_LIST,itemlist[i],MAP_ITEM_POS_X,NULL) ) {
			SDL_UnlockMutex(map_mutex);
			deep_free(itemlist);
			return NULL;
		}

		if( !entry_read_int(MAP_TABLE,map,&mapy,layer_name,MAP_ENTRY_ITEM_LIST,itemlist[i],MAP_ITEM_POS_Y,NULL) ) {
			SDL_UnlockMutex(map_mutex);
			deep_free(itemlist);
			return NULL;
		}

		if( x == mapx && y == mapy ) {
			id = itemlist[i];
			saved_item = strdup(itemlist[i]);
			break;
		}

		i++;
	}

	if( id == NULL ) {
		deep_free(itemlist);
		if( saved_item) {
			free(saved_item);
		}
		SDL_UnlockMutex(map_mutex);
		return NULL;
	}

	/* remove the item from the item list of the map */
	if(!entry_remove_group(MAP_TABLE,map,id,layer_name,MAP_ENTRY_ITEM_LIST,NULL)) {
		deep_free(itemlist);
		free(saved_item);
		SDL_UnlockMutex(map_mutex);
		return NULL;
	}

	deep_free(itemlist);

	SDL_UnlockMutex(map_mutex);

	/* Send network notifications */
	context_broadcast_map(map);

	return saved_item;
}
/******************************************
Add an item on map at given coordinate
return RET_FAIL if fails
******************************************/
int map_add_item(const char * map, int layer, const char * id, int x, int y)
{
	char layer_name[SMALL_BUF];

	if( x<0 || y<0 ) {
		return RET_FAIL;
	}

	sprintf(layer_name,"%s%d",MAP_KEY_LAYER,layer);

	SDL_LockMutex(map_mutex);

	if (!entry_write_int(MAP_TABLE,map,x,layer_name,MAP_ENTRY_ITEM_LIST,id,MAP_ITEM_POS_X, NULL) ) {
		entry_remove_group(MAP_TABLE,map,id,layer_name,MAP_ENTRY_ITEM_LIST,NULL);
		SDL_UnlockMutex(map_mutex);
		return RET_FAIL;
	}
	if (!entry_write_int(MAP_TABLE,map,y,layer_name,MAP_ENTRY_ITEM_LIST,id,MAP_ITEM_POS_Y, NULL) ) {
		entry_remove_group(MAP_TABLE,map,id,layer_name,MAP_ENTRY_ITEM_LIST,NULL);
		SDL_UnlockMutex(map_mutex);
		return RET_FAIL;
	}

	SDL_UnlockMutex(map_mutex);

	/* Send network notifications */
	context_broadcast_map(map);

	return 0;
}
/***********************************
Write a new tile into a map set
return RET_FAIL if fails
***********************************/
int map_set_tile(const char * map, int layer, const char * tile,int x, int y,int network_broadcast)
{
	char * previous_tile = NULL;
	int width = -1;
	int index;
	char layer_name[SMALL_BUF];

	if(map == NULL || tile == NULL) {
		return RET_FAIL;
	}

	if( x < 0 || y < 0) {
		return RET_FAIL;
	}

	sprintf(layer_name,"%s%d",MAP_KEY_LAYER,layer);

	/* Manage concurrent access to map files */
	SDL_LockMutex(map_mutex);

	/* read size of map grid */
	if(!entry_read_int(MAP_TABLE,map,&width,MAP_KEY_WIDTH,NULL)) {
		SDL_UnlockMutex(map_mutex);
		return RET_FAIL;
	}

	/* read layer's specific width */
	entry_read_int(MAP_TABLE,map,&width,layer_name,MAP_KEY_WIDTH,NULL);

	index = width * y + x;

	/* read previous map set */
	if(!entry_read_list_index(MAP_TABLE,map,&previous_tile,index,layer_name,MAP_KEY_SET,NULL)) {
		SDL_UnlockMutex(map_mutex);
		return RET_FAIL;
	}

	/* Do not change the tile if it is already the requested tile
	Avoid calling useless context_broadcast_file */
	if( strcmp(previous_tile, tile) == 0 ) {
		free(previous_tile);
		SDL_UnlockMutex(map_mutex);
		return RET_OK;
	}
	free(previous_tile);

	if( entry_write_list_index(MAP_TABLE, map, tile,index,layer_name,MAP_KEY_SET,NULL ) ) {
		if( network_broadcast ) {
			context_broadcast_map(map);
		}
	}

	SDL_UnlockMutex(map_mutex);

	return RET_OK;
}

/***********************************
Write a new tile type into a map file
return RET_FAIL if fails
***********************************/
int map_set_tile_type(const char * map, int layer, const char * type,int x, int y,int network_broadcast)
{
	char * previous_type = NULL;
	int width = -1;
	int index;
	char layer_name[SMALL_BUF];

	if(map == NULL || type == NULL) {
		return RET_FAIL;
	}

	if( x < 0 || y < 0) {
		return RET_FAIL;
	}

	sprintf(layer_name,"%s%d",MAP_KEY_LAYER,layer);

	/* Manage concurrent access to map files */
	SDL_LockMutex(map_mutex);

	/* read size of map grid */
	if(!entry_read_int(MAP_TABLE,map,&width,MAP_KEY_WIDTH,NULL)) {
		SDL_UnlockMutex(map_mutex);
		return RET_FAIL;
	}

	index = width * y + x;

	/* read previous map type */
	if( entry_read_list_index(MAP_TABLE,map,&previous_type, index,layer_name,MAP_KEY_TYPE,NULL) ) {
		/* Do not change the type if it already the requested type
		   Avoid calling useless context_broadcast_file */
		if( strcmp(previous_type, type) == 0 ) {
			free(previous_type);
			SDL_UnlockMutex(map_mutex);
			return RET_OK;
		}
		free(previous_type);
	}

	if( entry_write_list_index(MAP_TABLE, map, type,index, layer_name,MAP_KEY_TYPE,NULL) ) {
		if( network_broadcast ) {
			context_broadcast_map(map);
		}
	}

	SDL_UnlockMutex(map_mutex);

	return RET_OK;
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
return RET_FAIL if fails
***********************************/
int map_set_offscreen(const char * map, const char * script)
{
	int res;

	if(map == NULL || script == NULL) {
		return RET_FAIL;
	}

	/* Manage concurrent access to map files */
	SDL_LockMutex(map_mutex);

	res = entry_write_string(MAP_TABLE, map, script, MAP_OFFSCREEN,NULL);

	SDL_UnlockMutex(map_mutex);

	return res;
}

/***********************************
Set custom tiling of a map's layer
if layer == -1, set the map's grid custom columns
return RET_FAIL if fails
***********************************/
int map_set_custom_column(const char * map, int layer, int num, int width, int height)
{
	char layer_name[SMALL_BUF];
	char width_name[SMALL_BUF];
	char height_name[SMALL_BUF];
	int res;

	if(map == NULL) {
		return RET_FAIL;
	}

	/* Manage concurrent access to map files */
	SDL_LockMutex(map_mutex);

	if(num==0) {
		sprintf(width_name,"%s",MAP_KEY_COL_WIDTH);
		sprintf(height_name,"%s",MAP_KEY_COL_HEIGHT);
	} else {
		sprintf(width_name,"%s%d",MAP_KEY_COL_WIDTH,num);
		sprintf(height_name,"%s%d",MAP_KEY_COL_HEIGHT,num);
	}

	/* Map grid settings */
	if ( layer == -1 ) {
		res = entry_write_int(MAP_TABLE, map, width, width_name,NULL);
		if(res>-1) {
			res = entry_write_int(MAP_TABLE, map, height, height_name,NULL);
		}
	}
	/* Layer specific setting */
	else {
		sprintf(layer_name,"%s%d",MAP_KEY_LAYER,layer);
		res = entry_write_int(MAP_TABLE, map, width, layer_name,width_name,NULL);
		if(res>-1) {
			res = entry_write_int(MAP_TABLE, map, height, layer_name,height_name,NULL);
		}
	}

	SDL_UnlockMutex(map_mutex);

	return res;
}

/***********************************
Set custom tiling of a map's layer
if layer == -1, set the map's grid custom rows
return RET_FAIL if fails
***********************************/
int map_set_custom_row(const char * map, int layer, int num, int width, int height)
{
	char layer_name[SMALL_BUF];
	char width_name[SMALL_BUF];
	char height_name[SMALL_BUF];
	int res;

	if(map == NULL) {
		return RET_FAIL;
	}


	/* Manage concurrent access to map files */
	SDL_LockMutex(map_mutex);

	if(num==0) {
		sprintf(width_name,"%s",MAP_KEY_ROW_WIDTH);
		sprintf(height_name,"%s",MAP_KEY_ROW_HEIGHT);
	} else {
		sprintf(width_name,"%s%d",MAP_KEY_ROW_WIDTH,num);
		sprintf(height_name,"%s%d",MAP_KEY_ROW_HEIGHT,num);
	}

	if( layer == -1 ) {
		res = entry_write_int(MAP_TABLE, map, width, width_name,NULL);
		if(res>-1) {
			res = entry_write_int(MAP_TABLE, map, height, height_name,NULL);
		}
	} else {
		sprintf(layer_name,"%s%d",MAP_KEY_LAYER,layer);
		res = entry_write_int(MAP_TABLE, map, width, layer_name,width_name,NULL);
		if(res>-1) {
			res = entry_write_int(MAP_TABLE, map, height, layer_name,height_name,NULL);
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
	char * map_tile = NULL;
	int width;
	int height;
	char layer_name[SMALL_BUF];

	sprintf(layer_name,"%s%d",MAP_KEY_LAYER,layer);

	if(!entry_read_int(MAP_TABLE,map,&width,MAP_KEY_WIDTH,NULL)) {
		return NULL;
	}
	entry_read_int(MAP_TABLE,map,&width,layer_name,MAP_KEY_WIDTH,NULL);

	if(!entry_read_int(MAP_TABLE,map,&height,MAP_KEY_HEIGHT,NULL)) {
		return NULL;
	}
	entry_read_int(MAP_TABLE,map,&height,layer_name,MAP_KEY_HEIGHT,NULL);

	if( x<0 || y<0 || x >= width || y >= height ) {
		return NULL;
	}

	entry_read_list_index(MAP_TABLE,map,&map_tile,(width*y)+x,layer_name,MAP_KEY_SET,NULL);

	return map_tile;
}

/********************************************
 return the type of the tile on map layer at x,y
 Returned string MUST BE FREED
********************************************/
char * map_get_tile_type(const char * map, int layer, int x, int y)
{
	char * map_type = NULL;
	int width;
	int height;
	char layer_name[SMALL_BUF];


	if(!entry_read_int(MAP_TABLE,map,&width,MAP_KEY_WIDTH,NULL)) {
		return NULL;
	}

	if(!entry_read_int(MAP_TABLE,map,&height,MAP_KEY_HEIGHT,NULL)) {
		return NULL;
	}

	if( x<0 || y<0 || x >= width || y >= height ) {
		return NULL;
	}

	sprintf(layer_name,"%s%d",MAP_KEY_LAYER,layer);
	entry_read_list_index(MAP_TABLE,map,&map_type,(width*y)+x,layer_name,MAP_KEY_TYPE,NULL);

	if( map_type != NULL ) {
		return map_type;
	}

	return map_type;
}

/************************************************
 return an array of event id on given map at x,y
 This array MUST be freed by caller
************************************************/
char ** map_get_event(const char * map, int layer, int x, int y)
{
	char ** eventlist = NULL;
	char ** event_id = NULL;
	int i=0;
	int mapx;
	int mapy;
	int event_id_num = 0;
	char layer_name[SMALL_BUF];

	if( x<0 || y<0 ) {
		return NULL;
	}

	sprintf(layer_name,"%s%d",MAP_KEY_LAYER,layer);

	/* Manage concurrent acces to map files */
	SDL_LockMutex(map_mutex);
	/* Search the items on the specified tile */
	if(!entry_get_group_list(MAP_TABLE,map,&eventlist,layer_name,MAP_ENTRY_EVENT_LIST,NULL)) {
		SDL_UnlockMutex(map_mutex);
		return NULL;
	}

	while(eventlist[i] != NULL) {
		if( !entry_read_int(MAP_TABLE,map,&mapx,layer_name,MAP_ENTRY_EVENT_LIST,eventlist[i],MAP_EVENT_POS_X,NULL) ) {
			i++;
			continue;
		}

		if( !entry_read_int(MAP_TABLE,map,&mapy,layer_name,MAP_ENTRY_EVENT_LIST,eventlist[i],MAP_EVENT_POS_Y,NULL) ) {
			i++;
			continue;
		}

		if( x == mapx && y == mapy ) {
			event_id_num++;
			event_id=realloc(event_id,sizeof(char*)*(event_id_num+1));
			event_id[event_id_num-1] = strdup(eventlist[i]);
			event_id[event_id_num] = NULL;
		}

		i++;
	}
	deep_free(eventlist);

	SDL_UnlockMutex(map_mutex);

	return event_id;
}

/******************************************
 Add an event on map at given coordinate
 return NULL if fails
 return the event id is success
 the return event id must be freed by caller
***********************************************/
char * map_add_event(const char * map, int layer, const char * script, int x, int y )
{
	char * id;
	char layer_name[SMALL_BUF];

	if( x<0 || y<0 ) {
		return NULL;
	}

	sprintf(layer_name,"%s%d",MAP_KEY_LAYER,layer);

	/* Make sure the MAP_ENTRY_EVENT_LIST group exists */
	entry_group_create(MAP_TABLE,map,layer_name,MAP_ENTRY_EVENT_LIST,NULL);

	id = entry_get_unused_group(MAP_TABLE,map,layer_name,MAP_ENTRY_EVENT_LIST,NULL);
	if(id == NULL) {
		return NULL;
	}

	SDL_LockMutex(map_mutex);

	if (!entry_write_int(MAP_TABLE,map,x,layer_name,MAP_ENTRY_EVENT_LIST,id,MAP_EVENT_POS_X, NULL) ) {
		entry_remove_group(MAP_TABLE,map,id,layer_name,MAP_ENTRY_EVENT_LIST,NULL);
		free(id);
		SDL_UnlockMutex(map_mutex);
		return NULL;
	}
	if (!entry_write_int(MAP_TABLE,map,y,layer_name,MAP_ENTRY_EVENT_LIST,id,MAP_EVENT_POS_Y, NULL) ) {
		entry_remove_group(MAP_TABLE,map,id,layer_name,MAP_ENTRY_EVENT_LIST,NULL);
		free(id);
		SDL_UnlockMutex(map_mutex);
		return NULL;
	}

	if (!entry_write_string(MAP_TABLE,map,script,layer_name,MAP_ENTRY_EVENT_LIST,id,MAP_EVENT_SCRIPT, NULL) ) {
		entry_remove_group(MAP_TABLE,map,id,layer_name,MAP_ENTRY_EVENT_LIST,NULL);
		free(id);
		SDL_UnlockMutex(map_mutex);
		return NULL;
	}

	SDL_UnlockMutex(map_mutex);

	/* Send network notifications */
	context_broadcast_map(map);

	return id;
}

/******************************************
 Add a parameter to the given event
 return 0 if fails
***********************************************/
int map_add_event_param(const char * map, int layer, const char * event_id, const char * param)
{
	char layer_name[SMALL_BUF];

	sprintf(layer_name,"%s%d",MAP_KEY_LAYER,layer);

	/* Make sure the param list exists */
	entry_list_create(MAP_TABLE,map,layer_name,MAP_ENTRY_EVENT_LIST,event_id,MAP_EVENT_PARAM,NULL);

	return entry_add_to_list(MAP_TABLE,map,param,layer_name,MAP_ENTRY_EVENT_LIST,event_id,MAP_EVENT_PARAM,NULL);
}

/**********************************************
Delete an event on map at given coordinate
return RET_FAIL if fails
**********************************************/
int map_delete_event(const char * map, int layer, const char * script, int x, int y)
{
	char ** eventlist;
	int i=0;
	int mapx;
	int mapy;
	char * map_script = NULL;
	const char * id = NULL;
	char layer_name[SMALL_BUF];

	if( x<0 || y<0 ) {
		return RET_FAIL;
	}

	sprintf(layer_name,"%s%d",MAP_KEY_LAYER,layer);

	/* Manage concurrent acces to map files */
	SDL_LockMutex(map_mutex);
	/* Search events on the specified tile */
	if(!entry_get_group_list(MAP_TABLE,map,&eventlist,layer_name,MAP_ENTRY_EVENT_LIST,NULL)) {
		SDL_UnlockMutex(map_mutex);
		return RET_FAIL;
	}

	while(eventlist[i] != NULL) {
		if( !entry_read_int(MAP_TABLE,map,&mapx,layer_name,MAP_ENTRY_EVENT_LIST,eventlist[i],MAP_EVENT_POS_X,NULL) ) {
			i++;
			continue;
		}
		if( !entry_read_int(MAP_TABLE,map,&mapy,layer_name,MAP_ENTRY_EVENT_LIST,eventlist[i],MAP_EVENT_POS_Y,NULL) ) {
			i++;
			continue;
		}
		if( !entry_read_string(MAP_TABLE,map,&map_script,layer_name,MAP_ENTRY_EVENT_LIST,eventlist[i],MAP_EVENT_SCRIPT,NULL) ) {
			i++;
			continue;
		}
		if( x == mapx && y == mapy && !strcmp(map_script,script) ) {
			id = eventlist[i];
			free(map_script);
			break;
		}
		free(map_script);
		i++;
	}


	if( id == NULL ) {
		SDL_UnlockMutex(map_mutex);
		return RET_FAIL;
	}

	/* remove the event from the events list of the map */
	if(!entry_remove_group(MAP_TABLE,map,id,layer_name,MAP_ENTRY_EVENT_LIST,NULL)) {
		SDL_UnlockMutex(map_mutex);
		return RET_FAIL;
	}

	deep_free(eventlist);

	SDL_UnlockMutex(map_mutex);

	/* Send network notifications */
	context_broadcast_map(map);

	return RET_OK;
}

/************************************************
 return an array of character id on given map at x,y
 This array AND its content MUST be freed by caller
************************************************/
char ** map_get_character(const char * map, int x, int y)
{
	char ** character_list = NULL;
	int character_num = 0;
	context_t * ctx = context_get_first();

	if( x<0 || y<0 ) {
		return NULL;
	}

	context_lock_list();
	while(ctx!= NULL) {
		if(ctx->map == NULL) {
			ctx = ctx->next;
			continue;
		}
		if(ctx->pos_x == x && ctx->pos_y == y && !strcmp(ctx->map,map)) {
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

/************************************************
 return an array of item id on given map at x,y
 This array AND its content MUST be freed by caller
************************************************/
char ** map_get_item(const char * map, int layer, int map_x, int map_y)
{
	char ** item_id = NULL;
	char ** item_list= NULL;
	int item_num = 0;
	int x;
	int y;
	int i;
	char layer_name[SMALL_BUF];

	if( map_x<0 || map_y<0 ) {
		return NULL;
	}

	sprintf(layer_name,"%s%d",MAP_KEY_LAYER,layer);

	if(!entry_get_group_list(MAP_TABLE,map,&item_id,layer_name,MAP_ENTRY_ITEM_LIST,NULL)) {
		return NULL;
	}

	i=0;
	while( item_id[i] != NULL ) {
		if(!entry_read_int(MAP_TABLE,map,&x,layer_name,MAP_ENTRY_ITEM_LIST,item_id[i],MAP_ITEM_POS_X,NULL)) {
			i++;
			continue;
		}

		if(!entry_read_int(MAP_TABLE,map,&y,layer_name,MAP_ENTRY_ITEM_LIST,item_id[i],MAP_ITEM_POS_Y,NULL)) {
			i++;
			continue;
		}

		if ( x == map_x && y == map_y ) {
			item_num++;
			item_list=realloc(item_list,sizeof(char*)*(item_num+1));
			item_list[item_num-1] = strdup(item_id[i]);
			item_list[item_num] = NULL;
		}
		i++;
	}

	deep_free(item_id);

	return item_list;
}

/************************************************
Fill tx and ty with pixels coordinate of scpecified tile
tx and/or ty may be NULL
tx and ty are not modified on error
return RET_FAIL on error
************************************************/
int map_get_tile_coord(const char * map, int layer, int x, int y, int * tx, int * ty)
{
	layer_t default_layer;

	if( map == NULL ) {
		return RET_FAIL;
	}

	if( map_layer_update(map,NULL,&default_layer,DEFAULT_LAYER) == RET_FAIL) {
		return RET_FAIL;
	}

	if(tx) {	
		*tx = map_t2p_x(x,y,&default_layer);
	}
	if(ty) {
		*ty = map_t2p_y(x,y,&default_layer);
	}

	return RET_OK;
}

