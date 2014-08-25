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

#include "common.h"
#include <stdlib.h>
#include <string.h>
#include "../server/action.h"

/***********************************
Create a new map.
Return the name of the new map
*************************************/
char * map_new(int w,int h, int tile_w, int tile_h, const char * default_tile, const char * default_type)
{
	char * map_name;
	char ** tile_array;
	int i;
	char buf[SMALL_BUF];

	if( w<0 || h<0 ) {
		return NULL;
	}

	map_name = file_new(MAP_TABLE);
	if(map_name == NULL) {
		return NULL;
	}

	if (!entry_write_int(MAP_TABLE,map_name,w,MAP_KEY_HEIGHT, NULL) ) {
		free(map_name);
		return NULL;
	}
	if (!entry_write_int(MAP_TABLE,map_name,h,MAP_KEY_WIDTH, NULL) ) {
		free(map_name);
		return NULL;
	}
	if (!entry_write_int(MAP_TABLE,map_name,tile_w,MAP_KEY_TILE_WIDTH, NULL) ) {
		free(map_name);
		return NULL;
	}
	if (!entry_write_int(MAP_TABLE,map_name,tile_h,MAP_KEY_TILE_HEIGHT, NULL) ) {
		free(map_name);
		return NULL;
	}

	tile_array=malloc(((w*h)+1)*sizeof(char *));

	/* Write default tile */
	for(i=0; i<(w*h); i++) {
		tile_array[i] = (char *)default_tile;
	}
	tile_array[i] = NULL; /* End of list */
	
	sprintf(buf,"%s0",MAP_KEY_SET);
	if (!entry_write_list(MAP_TABLE,map_name,tile_array,buf, NULL) ) {
		free(map_name);
		return NULL;
	}

	/* Write default type */
	for(i=0; i<(w*h); i++) {
		tile_array[i] = (char *)default_type;
	}
	tile_array[i] = NULL; /* End of list */
	
	if (!entry_write_list(MAP_TABLE,map_name,tile_array,MAP_KEY_TYPE, NULL) ) {
		free(map_name);
		return NULL;
	}

	return map_name;
}

/***********************************
check if id is allowed to go on a tile
return TRUE if the context is allowed to go to the tile at coord x,y
*************************************/
int map_check_tile(context_t * ctx,char * id, const char * map, int x,int y)
{
	char * action;
	char sx[64];
	char sy[64];
	char * param[5];
	int res;
	char ** map_type;
	char * tile_type;
	char ** allowed_tile;
	int i=0;
	int width = 0;
	int height = 0;

	if(!entry_read_int(MAP_TABLE,map,&width,MAP_KEY_WIDTH,NULL)) {
		return FALSE;
	}
	if(!entry_read_int(MAP_TABLE,map,&height,MAP_KEY_HEIGHT,NULL)) {
		return FALSE;
	}

	if( x < 0 || y < 0 || x >= width || y >= height ) {
		return FALSE;
	}

	/* If there is a allowed_tile_script, run it */
	if(entry_read_string(CHARACTER_TABLE,id,&action, CHARACTER_KEY_ALLOWED_TILE_SCRIPT, NULL)) {
		param[0] = id;
		param[1] = (char *)map;
		sprintf(sx,"%d",x);
		sprintf(sy,"%d",y);
		param[2] = sx;
		param[3] = sy;
		param[4] = NULL;
		res = action_execute_script(ctx,action,param);
		free(action);
		return res;
	}

	/* Read tile list on this map */
	if(!entry_read_list(MAP_TABLE,map,&map_type,MAP_KEY_TYPE,NULL)) {
		return FALSE;
	}
	tile_type = map_type[(width*y)+x];

	/* If there is allowed_tile list, check it */
	if(entry_read_list(CHARACTER_TABLE,id,&allowed_tile,CHARACTER_KEY_ALLOWED_TILE,NULL)) {
		i=0;
		while( allowed_tile[i] != NULL ) {
			if( strcmp(allowed_tile[i], tile_type) == 0 ) {
				deep_free(allowed_tile);
				deep_free(map_type);
				return TRUE;
			}
			i++;
		}

		deep_free(allowed_tile);
		deep_free(map_type);
		return FALSE;
	}

	deep_free(map_type);
	/* Allow all tiles by default */
	return TRUE;
}

/**************************************
delete an item on context's map
**************************************/
char * map_delete_item(const char * map, int x, int y)
{
	char ** itemlist;
	int i=0;
	int mapx;
	int mapy;
	const char * id = NULL;
	char * saved_item = NULL;

	if( x<0 || y<0 ) {
		return NULL;
	}

	/* Manage concurrent acces to map files */
	SDL_LockMutex(map_mutex);
	/* Search the items on the specified tile */
	if(!entry_get_group_list(MAP_TABLE,map,&itemlist,MAP_ENTRY_ITEM_LIST,NULL)) {
		SDL_UnlockMutex(map_mutex);
		return NULL;
	}

	while(itemlist[i] != NULL) {
		if( !entry_read_int(MAP_TABLE,map,&mapx,MAP_ENTRY_ITEM_LIST,itemlist[i],MAP_ITEM_POS_X,NULL) ) {
			SDL_UnlockMutex(map_mutex);
			deep_free(itemlist);
			return NULL;
		}

		if( !entry_read_int(MAP_TABLE,map,&mapy,MAP_ENTRY_ITEM_LIST,itemlist[i],MAP_ITEM_POS_Y,NULL) ) {
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
	if(!entry_remove_group(MAP_TABLE,map,id,MAP_ENTRY_ITEM_LIST,NULL)) {
		deep_free(itemlist);
		free(saved_item);
		SDL_UnlockMutex(map_mutex);
		return NULL;
	}

	deep_free(itemlist);

	SDL_UnlockMutex(map_mutex);

	/* Send network notifications */
	context_broadcast_file(MAP_TABLE,map,TRUE);

	return saved_item;
}
/******************************************
Add an item on map at given coordinate
return -1 if fails
******************************************/
int map_add_item(const char * map, const char * id, int x, int y)
{

	if( x<0 || y<0 ) {
		return -1;
	}

	SDL_LockMutex(map_mutex);

	if (!entry_write_int(MAP_TABLE,map,x,MAP_ENTRY_ITEM_LIST,id,MAP_ITEM_POS_X, NULL) ) {
		entry_remove_group(MAP_TABLE,map,id,MAP_ENTRY_ITEM_LIST,NULL);
		SDL_UnlockMutex(map_mutex);
		return -1;
	}
	if (!entry_write_int(MAP_TABLE,map,y,MAP_ENTRY_ITEM_LIST,id,MAP_ITEM_POS_Y, NULL) ) {
		entry_remove_group(MAP_TABLE,map,id,MAP_ENTRY_ITEM_LIST,NULL);
		SDL_UnlockMutex(map_mutex);
		return -1;
	}

	SDL_UnlockMutex(map_mutex);

	/* Send network notifications */
	context_broadcast_file(MAP_TABLE,map,TRUE);

	return 0;
}
/***********************************
Write a new tile into a map set
return -1 if fails
***********************************/
int map_set_tile(const char * map,const char * tile,int x, int y, int level)
{
	char * previous_tile = NULL;
	int width = -1;
	int index;
	char buf[SMALL_BUF];

	if(map == NULL || tile == NULL) {
		return -1;
	}

	if( x < 0 || y < 0) {
		return -1;
	}

	/* Manage concurrent access to map files */
	SDL_LockMutex(map_mutex);

	/* read size of map */
	if(!entry_read_int(MAP_TABLE,map,&width,MAP_KEY_WIDTH,NULL)) {
		SDL_UnlockMutex(map_mutex);
		return -1;
	}

	index = width * y + x;

	/* read previous map set */
	sprintf(buf,"%s%d",MAP_KEY_SET,level);
	if(!entry_read_list_index(MAP_TABLE,map,&previous_tile, index,buf,NULL)) {
		SDL_UnlockMutex(map_mutex);
		return -1;
	}

	/* Do not change the tile if it is already the requested tile 
	Avoid calling useless context_broadcast_file */
	if( strcmp(previous_tile, tile) == 0 ) {
		free(previous_tile);
		SDL_UnlockMutex(map_mutex);
		return 0;
	}
	free(previous_tile);

	sprintf(buf,"%s%d",MAP_KEY_SET,level);
	if( entry_write_list_index(MAP_TABLE, map, tile,index, buf,NULL ) ) {
		context_broadcast_file(MAP_TABLE,map,TRUE);
	}

	SDL_UnlockMutex(map_mutex);

	return 0;
}

/***********************************
Write a new tile type into a map file
return -1 if fails
***********************************/
int map_set_tile_type(const char * map,const char * type,int x, int y)
{
	char * previous_type = NULL;
	int width = -1;
	int index;

	if(map == NULL || type == NULL) {
		return -1;
	}

	if( x < 0 || y < 0) {
		return -1;
	}

	/* Manage concurrent access to map files */
	SDL_LockMutex(map_mutex);

	/* read size of map */
	if(!entry_read_int(MAP_TABLE,map,&width,MAP_KEY_WIDTH,NULL)) {
		SDL_UnlockMutex(map_mutex);
		return -1;
	}

	index = width * y + x;

	/* read previous map type */
	if(!entry_read_list_index(MAP_TABLE,map,&previous_type, index,MAP_KEY_TYPE,NULL)) {
		SDL_UnlockMutex(map_mutex);
		return -1;
	}

	/* Do not change the type if it already the requested type 
	Avoid calling useless context_broadcast_file */
	if( strcmp(previous_type, type) == 0 ) {
		free(previous_type);
		SDL_UnlockMutex(map_mutex);
		return 0;
	}
	free(previous_type);

	if( entry_write_list_index(MAP_TABLE, map, type,index, MAP_KEY_TYPE,NULL ) ) {
		context_broadcast_file(MAP_TABLE,map,TRUE);
	}

	SDL_UnlockMutex(map_mutex);

	return 0;
}


/********************************************
 return the name of the tile on map at x,y
 return must be freed by caller
********************************************/
char * map_get_tile(const char * map,int x, int y, int level)
{
	char * map_tile = NULL;
	int width;
	int height;
	char buf[SMALL_BUF];


	if(!entry_read_int(MAP_TABLE,map,&width,MAP_KEY_WIDTH,NULL)) {
		return NULL;
	}

	if(!entry_read_int(MAP_TABLE,map,&height,MAP_KEY_HEIGHT,NULL)) {
		return NULL;
	}

	if( x<0 || y<0 || x >= width || y >= height ) {
		return NULL;
	}

	sprintf(buf,"%s%d",MAP_KEY_SET,level);
	entry_read_list_index(MAP_TABLE,map,&map_tile,(width*y)+x,buf,NULL);

	return map_tile;
}

/********************************************
 return the type of the tile on map at x,y
 Returned string MUST BE FREED
********************************************/
char * map_get_tile_type(const char * map,int x, int y)
{
	char * map_type = NULL;
	int width;
	int height;

	if(!entry_read_int(MAP_TABLE,map,&width,MAP_KEY_WIDTH,NULL)) {
		return NULL;
	}

	if(!entry_read_int(MAP_TABLE,map,&height,MAP_KEY_HEIGHT,NULL)) {
		return NULL;
	}
	
	if( x<0 || y<0 || x >= width || y >= height ) {
		return NULL;
	}

	entry_read_list_index(MAP_TABLE,map,&map_type,(width*y)+x,MAP_KEY_TYPE,NULL);
	
	return map_type;
}

/************************************************
 return an array of event id on given map at x,y
 This array MUST be freed by caller
************************************************/
char ** map_get_event(const char * map,int x, int y)
{
	char ** eventlist = NULL;
	char ** event_id = NULL;
	int i=0;
	int mapx;
	int mapy;
	int event_id_num = 0;

	if( x<0 || y<0 ) {
		return NULL;
	}

	/* Manage concurrent acces to map files */
	SDL_LockMutex(map_mutex);
	/* Search the items on the specified tile */
	if(!entry_get_group_list(MAP_TABLE,map,&eventlist,MAP_ENTRY_EVENT_LIST,NULL)) {
		SDL_UnlockMutex(map_mutex);
		return NULL;
	}

	while(eventlist[i] != NULL) {
		if( !entry_read_int(MAP_TABLE,map,&mapx,MAP_ENTRY_EVENT_LIST,eventlist[i],MAP_EVENT_POS_X,NULL) ) {
			i++;
			continue;
		}

		if( !entry_read_int(MAP_TABLE,map,&mapy,MAP_ENTRY_EVENT_LIST,eventlist[i],MAP_EVENT_POS_Y,NULL) ) {
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
char * map_add_event(const char * map, const char * script, int x, int y )
{
	char * id;

	if( x<0 || y<0 ) {
		return NULL;
	}

	/* Make sure the MAP_ENTRY_EVENT_LIST group exists */
	entry_group_create(MAP_TABLE,map,MAP_ENTRY_EVENT_LIST,NULL);

	id = entry_get_unused_group(MAP_TABLE,map,MAP_ENTRY_EVENT_LIST,NULL);
	if(id == NULL) {
		return NULL;
	}

	SDL_LockMutex(map_mutex);

	if (!entry_write_int(MAP_TABLE,map,x,MAP_ENTRY_EVENT_LIST,id,MAP_EVENT_POS_X, NULL) ) {
		entry_remove_group(MAP_TABLE,map,id,MAP_ENTRY_EVENT_LIST,NULL);
		free(id);
		SDL_UnlockMutex(map_mutex);
		return NULL;
	}
	if (!entry_write_int(MAP_TABLE,map,y,MAP_ENTRY_EVENT_LIST,id,MAP_EVENT_POS_Y, NULL) ) {
		entry_remove_group(MAP_TABLE,map,id,MAP_ENTRY_EVENT_LIST,NULL);
		free(id);
		SDL_UnlockMutex(map_mutex);
		return NULL;
	}

	if (!entry_write_string(MAP_TABLE,map,script,MAP_ENTRY_EVENT_LIST,id,MAP_EVENT_SCRIPT, NULL) ) {
		entry_remove_group(MAP_TABLE,map,id,MAP_ENTRY_EVENT_LIST,NULL);
		free(id);
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
int map_add_event_param(const char * map, const char * event_id, const char * param)
{
	/* Make sure the param list exists */
	entry_list_create(MAP_TABLE,map,MAP_ENTRY_EVENT_LIST,event_id,MAP_EVENT_PARAM,NULL);

	return entry_add_to_list(MAP_TABLE,map,param,MAP_ENTRY_EVENT_LIST,event_id,MAP_EVENT_PARAM,NULL);
}

/**********************************************
Delete an event on map at given coordinate
return -1 if fails
**********************************************/
int map_delete_event(const char * map, const char * script, int x, int y)
{
	char ** eventlist;
	int i=0;
	int mapx;
	int mapy;
	char * map_script = NULL;
	const char * id = NULL;

	if( x<0 || y<0 ) {
		return -1;
	}

	/* Manage concurrent acces to map files */
	SDL_LockMutex(map_mutex);
	/* Search events on the specified tile */
	if(!entry_get_group_list(MAP_TABLE,map,&eventlist,MAP_ENTRY_EVENT_LIST,NULL)) {
		SDL_UnlockMutex(map_mutex);
		return -1;
	}

	while(eventlist[i] != NULL) {
		if( !entry_read_int(MAP_TABLE,map,&mapx,MAP_ENTRY_EVENT_LIST,eventlist[i],MAP_EVENT_POS_X,NULL) ) {
			i++;
			continue;
		}
		if( !entry_read_int(MAP_TABLE,map,&mapy,MAP_ENTRY_EVENT_LIST,eventlist[i],MAP_EVENT_POS_Y,NULL) ) {
			i++;
			continue;
		}
		if( !entry_read_string(MAP_TABLE,map,&map_script,MAP_ENTRY_EVENT_LIST,eventlist[i],MAP_EVENT_SCRIPT,NULL) ) {
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
		return -1;
	}

	/* remove the event from the events list of the map */
	if(!entry_remove_group(MAP_TABLE,map,id,MAP_ENTRY_EVENT_LIST,NULL)) {
		SDL_UnlockMutex(map_mutex);
		return -1;
	}

	deep_free(eventlist);

	SDL_UnlockMutex(map_mutex);

	/* Send network notifications */
	context_broadcast_file(MAP_TABLE,map,TRUE);

	return 0;
}

/************************************************
 return an array of character id on given map at x,y
 This array AND its content MUST be freed by caller
************************************************/
char ** map_get_character(const char * map,int x, int y)
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
