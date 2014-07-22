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

#include <string.h>
#include <glib.h>
#include <stdlib.h>
#include <lualib.h>
#include <lauxlib.h>
#include <glib/gstdio.h>
#include "common.h"

context_t * context_list_start = NULL;

/* context_init
  Initialize a context_t struct
*/
void context_init(context_t * context)
{
	context->user_name = NULL;
	context->connected = FALSE;
	context->connection = NULL;
	context->input_stream = NULL;
	context->output_stream = NULL;
	context->data_connection = NULL;
	context->input_data_stream = NULL;
	context->output_data_stream = NULL;
	context->hostname = NULL;
	context->send_thread = NULL;

	context->render = NULL;
	context->window = NULL;

	context->character_name = NULL;
	context->map = NULL;
	context->map_x = -1;
	context->map_y = -1;
	context->tile_x = -1;
	context->tile_y = -1;

	context->pos_x = 0;
	context->pos_y = 0;
	context->cur_pos_x = 0;
	context->cur_pos_y = 0;
	context->old_pos_x = 0;
	context->old_pos_y = 0;
	context->pos_tick = 0;
	context->type = NULL;

	context->selection.id = NULL;
	context->selection.map_coord[0] = -1;
	context->selection.map_coord[1] = -1;
	context->selection.map = NULL;
	context->selection.inventory = NULL;
	context->selection.equipment = NULL;

	context->id = NULL;
	context->prev_map = NULL;
	context->change_map = 0;
	context->luaVM = NULL;
	context->cond = NULL;
	context->cond_mutex = NULL;
	context->previous = NULL;
	context->next = NULL;
}

/* Add a new context to the list */
context_t * context_new(void)
{
	context_t * ctx;

	SDL_LockMutex(context_list_mutex);
	if ( context_list_start == NULL ) {
		context_list_start = malloc(sizeof(context_t));
		memset(context_list_start,0,sizeof(context_t));
		context_init(context_list_start);
		SDL_UnlockMutex(context_list_mutex);
		return context_list_start;
	}

	ctx = context_list_start;
	while( ctx->next != NULL ) {
		ctx = ctx->next;
	}

	ctx->next = malloc(sizeof(context_t));
	memset(ctx->next,0,sizeof(context_t));
	context_init(ctx->next);
	ctx->next->previous = ctx;
	SDL_UnlockMutex(context_list_mutex);
	return ctx->next;
}

/* context_free
  Deep free of a context_t struct
*/
void context_free(context_t * context)
{
	char filename[512] = "";
	int delete_file = TRUE;
	context_t * ctx;

	SDL_LockMutex(context_list_mutex);

	
// We should not erase the file's data from memory until it's dumped to disk, so comment this line
//	entry_destroy(context->id);

	free(context->user_name);
	context->user_name = NULL;
	context->connected = FALSE;
	if( context->connection != NULL) {
		g_io_stream_close((GIOStream *)context->connection,NULL,NULL);
		delete_file = FALSE;
	}
	context->connection = NULL;
	if( context->data_connection != NULL) {
		g_io_stream_close((GIOStream *)context->data_connection,NULL,NULL);
		delete_file = FALSE;
	}
	context->data_connection = NULL;
	context->input_stream = NULL;
	context->output_stream = NULL;
	context->input_data_stream = NULL;
	context->output_data_stream = NULL;
	free(context->hostname);
	context->hostname = NULL;
	context->send_thread = NULL;
	free(context->character_name);
	context->character_name = NULL;
	free(context->map);
	context->map = NULL;
	free(context->type);
	context->type = NULL;
	context->selection.id = NULL;
	context->selection.map_coord[0] = -1;
	context->selection.map_coord[1] = -1;
	context->selection.map = NULL;
	free(context->selection.inventory);
	context->selection.inventory = NULL;
	free(context->selection.equipment);
	context->selection.equipment = NULL;
	free(context->id);
	context->id = NULL;
	free(context->prev_map);
	context->prev_map = NULL;
	if( context->luaVM != NULL) {
		lua_close(context->luaVM);
	}
	if( context->cond != NULL) {
		SDL_DestroyCond(context->cond);
	}
	if( context->cond_mutex != NULL) {
		SDL_DestroyMutex(context->cond_mutex);
	}

	/* First context of the list */
	if( context->previous == NULL ) {
		context_list_start = context->next;
		if( context->next != NULL) {
			context->next->previous = NULL;
		}
	} else {
		context->previous->next = context->next;
		if( context->next != NULL) {
			context->next->previous = context->previous;
		}
	}

	/* Remove this context if it was selected */
	ctx = context_list_start;
	while( ctx != NULL ) {
		if( context->id && ctx->selection.id ) {
			if (strcmp(context->id,ctx->selection.id)==0) {
				ctx->selection.id = NULL;
			}
		}
		ctx = ctx->next;
	}

	free(context);

	SDL_UnlockMutex(context_list_mutex);

	if(delete_file) {
		strcat(filename,getenv("HOME"));
		strcat(filename,"/");
		strcat(filename,base_directory);
		strcat(filename,"/");
		strcat(filename,CHARACTER_TABLE);
		strcat(filename,"/");
		strcat(filename,context->id);

		unlink(filename);
	}
}

void context_lock_list()
{
	SDL_LockMutex(context_list_mutex);
}

void context_unlock_list()
{
	SDL_UnlockMutex(context_list_mutex);
}

context_t * context_get_list_first()
{
	return context_list_start;
}

/* context_set_username
  Set user_name

  Returns FALSE if error
*/
int context_set_username(context_t * context, const char * name)
{
	SDL_LockMutex(context_list_mutex);

	free( context->user_name );
	context->user_name = strdup(name);
	if( context->user_name == NULL ) {
		SDL_UnlockMutex(context_list_mutex);
		return FALSE;
	}

	SDL_UnlockMutex(context_list_mutex);
	return TRUE;
}

/* context_set_connected
  Set connected flag
*/
void context_set_connected(context_t * context, int connected)
{
	SDL_LockMutex(context_list_mutex);
	context->connected = connected;
	SDL_UnlockMutex(context_list_mutex);
}
/* context_get_connected
  Get connection flag
*/
int context_get_connected(context_t * context)
{
	int conn = FALSE;

	SDL_LockMutex(context_list_mutex);
	conn = context->connected;
	SDL_UnlockMutex(context_list_mutex);

	return conn;
}

void context_set_input_stream(context_t * context, GInputStream * stream)
{
	SDL_LockMutex(context_list_mutex);
	context->input_stream = stream;
	SDL_UnlockMutex(context_list_mutex);
}

GInputStream * context_get_input_stream(context_t * context)
{
	GInputStream * conn = NULL;

	SDL_LockMutex(context_list_mutex);
	conn = context->input_stream;
	SDL_UnlockMutex(context_list_mutex);

	return conn;
}

void context_set_output_stream(context_t * context, GOutputStream * stream)
{
	SDL_LockMutex(context_list_mutex);
	context->output_stream = stream;
	SDL_UnlockMutex(context_list_mutex);
}

GOutputStream * context_get_output_stream(context_t * context)
{
	GOutputStream * conn = NULL;

//FIXME
//	SDL_LockMutex(context_list_mutex);
	conn = context->output_stream;
//	SDL_UnlockMutex(context_list_mutex);

	return conn;
}

void context_set_connection(context_t * context, GSocketConnection * connection)
{
	SDL_LockMutex(context_list_mutex);
	context->connection = connection;
	SDL_UnlockMutex(context_list_mutex);
}

GSocketConnection * context_get_connection(context_t * context)
{
	GSocketConnection * conn = NULL;

	SDL_LockMutex(context_list_mutex);
	conn = context->connection;
	SDL_UnlockMutex(context_list_mutex);

	return conn;
}
/* context_set_character_name
  Set name

  Returns FALSE if error
*/
int context_set_character_name(context_t * context, const char * name)
{
	int ret = TRUE;

	SDL_LockMutex(context_list_mutex);
	free( context->character_name );
	context->character_name = strdup(name);
	if( context->character_name == NULL ) {
		ret = FALSE;
	}
	SDL_UnlockMutex(context_list_mutex);

	return ret;
}
/* context_set_map
  Set map name

  Returns FALSE if error
*/
static int _context_set_map(context_t * context, const char * map)
{
	int map_x;
	int map_y;

	if(context->prev_map != NULL) {
		if(!strcmp(context->map,map)) {
			return TRUE;
		}
		free( context->prev_map );
	}

	if(!read_int(MAP_TABLE,map,&map_x,MAP_KEY_SIZE_X,NULL)) {
		return FALSE;
	}
	if(!read_int(MAP_TABLE,map,&map_y,MAP_KEY_SIZE_Y,NULL)) {
		return FALSE;
	}

	if(context->map) {
		context->prev_map = strdup(context->map);
		free( context->map );
	}

	context->map = strdup(map);
	if( context->map == NULL ) {
		return FALSE;
	}
	context->map_x = map_x;
	context->map_y = map_y;
	context->change_map = 1;

	return TRUE;
}
int context_set_map(context_t * context, const char * map)
{
	int ret;

	SDL_LockMutex(context_list_mutex);
	ret = _context_set_map(context,map);
	SDL_UnlockMutex(context_list_mutex);

	return ret;
}
/* context_set_type
  Set type name

  Returns FALSE if error
*/
int context_set_type(context_t * context, const char * type)
{
	int ret = TRUE;

	SDL_LockMutex(context_list_mutex);
	free( context->type );
	context->type = strdup(type);
	if( context->type == NULL ) {
		ret = FALSE;
	}
	SDL_UnlockMutex(context_list_mutex);

	return ret;
}

/* context_set_pos_x
  Set pos_x
*/
void context_set_pos_x(context_t * context, unsigned int pos)
{
	SDL_LockMutex(context_list_mutex);
	context->pos_x = pos;
	SDL_UnlockMutex(context_list_mutex);
}

/* context_set_pos_y
  Set pos_y
*/
void context_set_pos_y(context_t * context, unsigned int pos)
{
	SDL_LockMutex(context_list_mutex);
	context->pos_y = pos;
	SDL_UnlockMutex(context_list_mutex);
}

/* context_set_tile_x
  Set tile_x
*/
void context_set_tile_x(context_t * context, unsigned int pos)
{
	SDL_LockMutex(context_list_mutex);
	context->tile_x = pos;
	SDL_UnlockMutex(context_list_mutex);
}

/* context_set_tile_y
  Set tile_y
*/
void context_set_tile_y(context_t * context, unsigned int pos)
{
	SDL_LockMutex(context_list_mutex);
	context->tile_y = pos;
	SDL_UnlockMutex(context_list_mutex);
}

/* context_set_id
  Set id

  Returns FALSE if error
*/
int context_set_id(context_t * context, const char * name)
{
	SDL_LockMutex(context_list_mutex);

	free( context->id );
	context->id = strdup(name);
	if( context->id == NULL ) {
		SDL_UnlockMutex(context_list_mutex);
		return FALSE;
	}

	SDL_UnlockMutex(context_list_mutex);
	return TRUE;
}

#ifdef SERVER
void register_lua_functions( context_t * context);
#else
/* Client do not use LUA for now */
void register_lua_functions( context_t * context) {}
#endif
void context_new_VM(context_t * context)
{
	SDL_LockMutex(context_list_mutex);
	context->luaVM = lua_open();
	lua_baselibopen(context->luaVM);
	lua_tablibopen(context->luaVM);
	lua_iolibopen(context->luaVM);
	lua_strlibopen(context->luaVM);
	lua_mathlibopen(context->luaVM);

	register_lua_functions(context);
	SDL_UnlockMutex(context_list_mutex);
}
/*******************************
Update the memory context by reading the client's character data file on disk
*******************************/
int context_update_from_file(context_t * context)
{
	/* Don't call context_set_* functions here to avoid inter-blocking */

	const char * result;
	int ret;

	SDL_LockMutex(context_list_mutex);

	if( context->id == NULL ) {
		SDL_UnlockMutex(context_list_mutex);
		return FALSE;
	}

	if(!read_string(CHARACTER_TABLE,context->id,&result, CHARACTER_KEY_NAME,NULL)) {
		SDL_UnlockMutex(context_list_mutex);
		return FALSE;
	}

	free( context->character_name );
	context->character_name = strdup(result);
	if( context->character_name == NULL ) {
		SDL_UnlockMutex(context_list_mutex);
		return FALSE;
	}

	if(!read_string(CHARACTER_TABLE,context->id,&result, CHARACTER_KEY_TYPE,NULL)) {
		SDL_UnlockMutex(context_list_mutex);
		return FALSE;
	}

	free( context->type );
	context->type = strdup(result);
	if( context->type == NULL ) {
		SDL_UnlockMutex(context_list_mutex);
		return FALSE;
	}

	if(!read_string(CHARACTER_TABLE,context->id,&result, CHARACTER_KEY_MAP,NULL)) {
		SDL_UnlockMutex(context_list_mutex);
		return FALSE;
	}

	free( context->map );
	ret = _context_set_map(context, result);
	if( ret == FALSE ) {
		SDL_UnlockMutex(context_list_mutex);
		return FALSE;
	}

	if(!read_int(CHARACTER_TABLE,context->id,&context->pos_x, CHARACTER_KEY_POS_X,NULL)) {
		SDL_UnlockMutex(context_list_mutex);
		return FALSE;
	}

	if(!read_int(CHARACTER_TABLE,context->id,&context->pos_y, CHARACTER_KEY_POS_Y,NULL)) {
		SDL_UnlockMutex(context_list_mutex);
		return FALSE;
	}

	SDL_UnlockMutex(context_list_mutex);
	return TRUE;
}

/*******************************
Write a context to server's disk
*******************************/
int context_write_to_file(context_t * context)
{
	SDL_LockMutex(context_list_mutex);

	if( context->id == NULL ) {
		SDL_UnlockMutex(context_list_mutex);
		return FALSE;
	}

	write_string(CHARACTER_TABLE, context->id,context->type,CHARACTER_KEY_TYPE,NULL);
	write_string(CHARACTER_TABLE, context->id,context->map,CHARACTER_KEY_MAP, NULL);


	write_int(CHARACTER_TABLE, context->id,context->pos_x,CHARACTER_KEY_POS_X, NULL);

	write_int(CHARACTER_TABLE, context->id,context->pos_y,CHARACTER_KEY_POS_Y, NULL);

	SDL_UnlockMutex(context_list_mutex);
	return TRUE;
}

/*******************************
Find a context in memroy from its id
*******************************/
context_t * context_find(const char * id)
{
	context_t * ctx;

	ctx = context_list_start;

	while(ctx != NULL) {
		if( ctx->id ) {
			if( strcmp(ctx->id,id) == 0 ) {
				return ctx;
			}
		}

		ctx = ctx->next;
	}

	return NULL;
}

/*******************************
Update the context by decoding the received frame
Called by server
*******************************/
int context_update_from_network_frame(context_t * context, char * frame)
{
	char * data = frame;

	SDL_LockMutex(context_list_mutex);

	if( context->user_name ) {
		free( context->user_name );
	}
	context->user_name = strdup(data);

	data += (strlen(data)+1);

	if( context->character_name ) {
		free( context->character_name );
	}
	context->character_name = strdup(data);
	data += (strlen(data)+1);

	_context_set_map(context,data);
	data += (strlen(data)+1);

	context->connected = atoi(data);
	data += (strlen(data)+1);

	context->pos_x = atoi(data);
	data += (strlen(data)+1);

	context->pos_y = atoi(data);
	data += (strlen(data)+1);

	if( context->type ) {
		free( context->type );
	}
	context->type = strdup(data);
	data += (strlen(data)+1);

	context->tile_x = atoi(data);
	data += (strlen(data)+1);

	context->tile_y = atoi(data);
	data += (strlen(data)+1);

	if( context->id ) {
		free( context->id );
	}
	context->id = strdup(data);
	data += (strlen(data)+1);

	if( context->selection.id ) {
		free( context->selection.id );
	}
	if( data[0] != 0 ) {
		context->selection.id = strdup(data);
	} else {
		context->selection.id = NULL;
	}
	data += (strlen(data)+1);

	context->selection.map_coord[0] = atoi(data);
	data += (strlen(data)+1);

	context->selection.map_coord[1] = atoi(data);
	data += (strlen(data)+1);

	if( context->selection.map ) {
		free( context->selection.map );
	}
	if( data[0] != 0 ) {
		context->selection.map = strdup(data);
	} else {
		context->selection.map = NULL;
	}
	data += (strlen(data)+1);

	if( context->selection.inventory ) {
		free( context->selection.inventory );
	}
	if( data[0] != 0 ) {
		context->selection.inventory = strdup(data);
	} else {
		context->selection.inventory = NULL;
	}
	data += (strlen(data)+1);

	if( context->selection.equipment ) {
		free( context->selection.equipment );
	}
	if( data[0] != 0 ) {
		context->selection.equipment = strdup(data);
	} else {
		context->selection.equipment = NULL;
	}
	data += (strlen(data)+1);

	SDL_UnlockMutex(context_list_mutex);

	return TRUE;
}

/* Spread the data of a context to all connected context */
void context_spread(context_t * context)
{
	context_t * ctx = NULL;

	SDL_LockMutex(context_list_mutex);

	ctx = context_list_start;

	if( ctx == NULL ) {
		SDL_UnlockMutex(context_list_mutex);
		return;
	}

	do {
		/* Skip NPC */
		if( ctx->output_stream == NULL ) {
			continue;
		}

		/* Skip if not on the same map or previous map */
		if( ctx->map && context->map && context->prev_map) {
			if( strcmp(context->map,ctx->map) != 0 &&
					strcmp(context->prev_map,ctx->map) != 0 ) {
				continue;
			}
		}

		/* Skip if the player has not selected its character (not yet entered in the game)  */
		if( ctx->id == NULL ) {
			continue;
		}

		network_send_context_to_context(ctx, context);


	} while( (ctx=ctx->next)!= NULL );

	/* The existing context on the previous map should have
	been deleted, we don't need this anymore -> this will
	generate less network request */
	free(context->prev_map);
	context->prev_map = NULL;

	SDL_UnlockMutex(context_list_mutex);
}

/*if "map" == NULL : server sends a message to all connected client
 if "map" != NULL : server sends a message to all connected clients on the map
*/
void context_broadcast_text(const char * map, const char * text)
{
	context_t * ctx = NULL;

	SDL_LockMutex(context_list_mutex);

	ctx = context_list_start;

	if( ctx == NULL ) {
		SDL_UnlockMutex(context_list_mutex);
		return;
	}

	do {
		/* Skip NPC */
		if( ctx->output_stream == NULL ) {
			continue;
		}

		if(map) {
			/* Skip if not on the same map */
			if( strcmp(map,ctx->map) != 0 ) {
				continue;
			}

			/* Skip if the player has not selected its character */
			if( ctx->id == NULL ) {
				continue;
			}
		}

		network_send_text(ctx->id, text);

	} while( (ctx=ctx->next)!= NULL );

	SDL_UnlockMutex(context_list_mutex);
}

/* Send the data of all existing context to the passed context */
/* Useful at start time */
void context_request_other_context(context_t * context)
{
	context_t * ctx = NULL;

	SDL_LockMutex(context_list_mutex);

	ctx = context_list_start;

	if( ctx == NULL ) {
		SDL_UnlockMutex(context_list_mutex);
		return;
	}

	do {
		/* Skip the calling context */
		if( context == ctx ) {
			continue;
		}

		/* Skip if not on the same map */
		if( ctx->map ) {
			if( strcmp(context->map,ctx->map) != 0 ) {
				continue;
			}
		}

		network_send_context_to_context(context, ctx);

	} while( (ctx=ctx->next)!= NULL );

	SDL_UnlockMutex(context_list_mutex);
}

/* Called from client */
void context_add_or_update_from_network_frame(context_t * context,char * data)
{
	context_t * ctx = NULL;
	char * user_name = NULL;
	char * name = NULL;
	char * map = NULL;
	int connected;
	int pos_x;
	int pos_y;
	int tile_x;
	int tile_y;
	char * type = NULL;
	char * id = NULL;

	/* First decode the data */

	user_name = strdup(data);
	data += (strlen(data)+1);

	name = strdup(data);
	data += (strlen(data)+1);

	map = strdup(data);
	data += (strlen(data)+1);

	connected = atoi(data);
	data += (strlen(data)+1);

	pos_x = atoi(data);
	data += (strlen(data)+1);

	pos_y = atoi(data);
	data += (strlen(data)+1);

	type = strdup(data);
	data += (strlen(data)+1);

	tile_x = atoi(data);
	data += (strlen(data)+1);

	tile_y = atoi(data);
	data += (strlen(data)+1);

	id = strdup(data);
	data += (strlen(data)+1);

	/* search for this context */
	SDL_LockMutex(context_list_mutex);
	ctx = context_list_start;

	while( ctx != NULL ) {
		if( strcmp( id, ctx->id) == 0 ) {
			if( connected != FALSE ) {
				wlog(LOGDEBUG,"Updating context %s / %s",user_name,name);
				/* do not call context_set_* function since we already have the lock */
				_context_set_map(ctx,map);

				ctx->pos_x = pos_x;
				ctx->pos_y = pos_y;

				ctx->tile_x = tile_x;
				ctx->tile_y = tile_y;

				free(ctx->type);
				ctx->type = type;

				free(user_name);
				free(name);
				free(id);

				SDL_UnlockMutex(context_list_mutex);
			} else {
				wlog(LOGDEBUG,"Deleting context %s / %s",user_name,name);
				/* Delete selection if it was selected */
				if( context->selection.id != NULL ) {
					if( strcmp(context->selection.id, id) == 0 ) {
						context->selection.id = NULL;
						network_send_context(context);
					}

				}
				SDL_UnlockMutex(context_list_mutex);
				context_free(ctx);
			}
			return;
		}
		ctx = ctx->next;
	}

	SDL_UnlockMutex(context_list_mutex);

	wlog(LOGDEBUG,"Creating context %s / %s",user_name,name);
	ctx = context_new();
	context_set_username(ctx,user_name);
	context_set_character_name(ctx,name);
	context_set_map(ctx,map);
	context_set_type(ctx,type);
	context_set_pos_x(ctx,pos_x);
	context_set_pos_y(ctx,pos_y);
	context_set_tile_x(ctx,tile_x);
	context_set_tile_y(ctx,tile_y);
	context_set_id(ctx,id);

	free(user_name);
	free(name);
	free(map);
	free(type);
}

/* Broadcast upload of a file to all connected context */
void context_broadcast_file(const char * table, const char * file, int same_map_only)
{
	context_t * ctx = NULL;
	char filename[512] = "";

	SDL_LockMutex(context_list_mutex);

	ctx = context_list_start;

	if( ctx == NULL ) {
		SDL_UnlockMutex(context_list_mutex);
		return;
	}

	strcat(filename,table);
	strcat(filename,"/");
	strcat(filename,file);

	do {
		/* Skip if not connected (NPC) */
		if( ctx->output_stream == NULL ) {
			continue;
		}
		/* Skip if not on the same map */
		if( same_map_only && ctx->map) {
			if( strcmp(file,ctx->map) != 0 ) {
				continue;
			}
		}

		network_send_file(ctx,filename);

	} while( (ctx=ctx->next)!= NULL );

	SDL_UnlockMutex(context_list_mutex);
}

/* Return the distance between two contexts */
int context_distance(context_t * ctx1, context_t * ctx2)
{
	int distx;
	int disty;

	distx = ctx1->pos_x - ctx2->pos_x;
	if(distx < 0 ) {
		distx = -distx;
	}
	disty = ctx1->pos_y - ctx2->pos_y;
	if(disty < 0 ) {
		disty = -disty;
	}

	return (distx>disty?distx:disty);
}

/* Reset all contexts position information used for smooth animation */
/* Called on screen switch */
void context_reset_all_position()
{
	context_t * ctx = context_get_list_first();

	SDL_LockMutex(context_list_mutex);
	while(ctx != NULL ) {
		ctx->old_pos_x = ctx->cur_pos_x;
		ctx->old_pos_y = ctx->cur_pos_y;
		ctx->pos_tick = 0;
		ctx = ctx->next;
	}
	SDL_UnlockMutex(context_list_mutex);
}
