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

#include <string.h>
#include <stdlib.h>
#include <lualib.h>
#include <lauxlib.h>
#include <unistd.h>
#include "common.h"

context_t * context_list_start = NULL;

/***********************
***********************/
void context_unlock_list()
{
	SDL_UnlockMutex(context_list_mutex);
}

/***********************
***********************/
context_t * context_get_first()
{
	return context_list_start;
}
/*************************************
context_init
Initialize a context_t struct
*************************************/
void context_init(context_t * context)
{
	context->user_name = NULL;
	context->connected = false;
	context->in_game = false;
	context->socket = 0;
	context->socket_data = 0;
	context->hostname = NULL;
	context->send_mutex = SDL_CreateMutex();

	context->render = NULL;
	context->window = NULL;

	context->character_name = NULL;
	context->map = NULL;

	context->pos_tx = 0;
	context->pos_ty = 0;
	context->prev_pos_tx = 0;
	context->prev_pos_ty = 0;
	context->cur_pos_px = 0;
	context->cur_pos_py = 0;
	context->pos_changed = FALSE;
	context->move_start_tick = 0;
	context->animation_tick = 0;
	context->type = NULL;

	context->selection.id = strdup("");
	context->selection.map_coord[0] = -1;
	context->selection.map_coord[1] = -1;
	context->selection.map = strdup("");
	context->selection.inventory = strdup("");
	context->selection.equipment = strdup("");

	context->id = NULL;
	context->prev_map = NULL;
	context->change_map = false;
	context->luaVM = NULL;
	context->cond = NULL;
	context->cond_mutex = NULL;
	context->orientation = 0;
	context->direction = 0;
	context->next_execution_time = 0;

	context->previous = NULL;
	context->next = NULL;
}

/**************************************
Add a new context to the list
**************************************/
context_t * context_new(void)
{
	context_t * ctx;

	context_lock_list();
	if ( context_list_start == NULL ) {
		context_list_start = malloc(sizeof(context_t));
		memset(context_list_start,0,sizeof(context_t));
		context_init(context_list_start);
		context_unlock_list();
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
	context_unlock_list();
	return ctx->next;
}

/*************************************
context_free
Deep free of a context_t struct
*************************************/
void context_free(context_t * context)
{
	context_t * ctx;

	context_lock_list();

	if( context->user_name ) {
		free(context->user_name);
	}
	context->user_name = NULL;
	context->in_game = false;
	context->connected = false;
	if( context->socket != 0) {
		SDLNet_TCP_Close(context->socket);
	}
	context->socket = 0;
	if( context->socket_data != 0) {
		SDLNet_TCP_Close(context->socket_data);
	}
	context->socket_data = 0;
	SDL_DestroyMutex(context->send_mutex);

	if( context->hostname ) {
		free(context->hostname);
	}
	context->hostname = NULL;
	if( context->character_name ) {
		free(context->character_name);
	}
	context->character_name = NULL;
	if( context->map ) {
		free(context->map);
	}
	context->map = NULL;
	if( context->type ) {
		free(context->type);
	}
	context->type = NULL;
	if( context->selection.id ) {
		free(context->selection.id);
	}
	context->selection.id = NULL;
	context->selection.map_coord[0] = -1;
	context->selection.map_coord[1] = -1;
	if( context->selection.map ) {
		free(context->selection.map);
	}
	context->selection.map = NULL;
	if( context->selection.inventory ) {
		free(context->selection.inventory);
	}
	context->selection.inventory = NULL;
	if( context->selection.equipment ) {
		free(context->selection.equipment);
	}
	context->selection.equipment = NULL;
	if( context->prev_map ) {
		free(context->prev_map);
	}
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

	/* Remove this context from other context's selection */
	ctx = context_list_start;
	while( ctx != NULL ) {
		if( context->id && ctx->selection.id ) {
			if (strcmp(context->id,ctx->selection.id)==0) {
				ctx->selection.id = NULL;
			}
		}
		ctx = ctx->next;
	}

	if( context->id ) {
		free(context->id);
	}
	context->id = NULL;

	/* Remove this context from the list */
	if( context == context_get_first() ) {
		context_list_start = context->next;
	} else {
		ctx = context_list_start;
		while( ctx != NULL ) {
			if( ctx->next == context ) {
				ctx->next = context->next;
				break;
			}
			ctx = ctx->next;
		}
	}

	free(context);

	context_unlock_list();
}

/***********************
***********************/
void context_lock_list()
{
	SDL_LockMutex(context_list_mutex);
}

/***********************
***********************/
context_t * context_get_player()
{
	return context_list_start;
}
/**************************
  Returns false if error
**************************/
int context_set_hostname(context_t * context, const char * name)
{
	context_lock_list();

	if( context->hostname ) {
		free( context->hostname );
	}

	context->hostname = strdup(name);
	if( context->hostname == NULL ) {
		context_unlock_list();
		return false;
	}

	context_unlock_list();
	return true;
}

/**************************
  Returns false if error
**************************/
int context_set_username(context_t * context, const char * name)
{
	context_lock_list();

	free( context->user_name );
	context->user_name = strdup(name);
	if( context->user_name == NULL ) {
		context_unlock_list();
		return false;
	}

	context_unlock_list();
	return true;
}

/**************************************
**************************************/
void context_set_in_game(context_t * context, int in_game)
{
	context_lock_list();
	context->in_game = in_game;
	context_unlock_list();
}

/**************************************
**************************************/
int context_get_in_game(context_t * context)
{
	int in_game = 0;

	context_lock_list();
	in_game = context->in_game;
	context_unlock_list();

	return in_game;
}

/**************************************
**************************************/
void context_set_connected(context_t * context, int connected)
{
	context_lock_list();
	context->connected = connected;
	context_unlock_list();
}

/**************************************
**************************************/
int context_get_connected(context_t * context)
{
	int conn = 0;

	context_lock_list();
	conn = context->connected;
	context_unlock_list();

	return conn;
}

/**************************************
**************************************/
void context_set_socket(context_t * context, TCPsocket socket)
{
	context_lock_list();
	context->socket = socket;
	context_unlock_list();
}

/**************************************
**************************************/
TCPsocket context_get_socket(context_t * context)
{
	TCPsocket socket = 0;

	context_lock_list();
	socket = context->socket;
	context_unlock_list();

	return socket;
}

/**************************************
**************************************/
void context_set_socket_data(context_t * context, TCPsocket socket)
{
	context_lock_list();
	context->socket_data = socket;
	context_unlock_list();
}

/**************************************
**************************************/
TCPsocket context_get_socket_data(context_t * context)
{
	TCPsocket socket = 0;

	context_lock_list();
	socket = context->socket_data;
	context_unlock_list();

	return socket;
}

/**************************************
Returns false if error
**************************************/
int context_set_character_name(context_t * context, const char * name)
{
	int ret = true;

	context_lock_list();
	free( context->character_name );
	context->character_name = strdup(name);
	if( context->character_name == NULL ) {
		ret = false;
	}
	context_unlock_list();

	return ret;
}

/**************************************
Returns false if error
**************************************/
static int _context_set_map(context_t * context, const char * map)
{
	if(context->prev_map != NULL) {
		if(!strcmp(context->map,map)) {
			return true;
		}
		free( context->prev_map );
		context->prev_map = NULL;
	}

	if(context->map) {
		context->prev_map = strdup(context->map);
		free( context->map );
	}

	context->map = strdup(map);
	if( context->map == NULL ) {
		return false;
	}
	context->change_map = true;

	return true;
}

/**************************************
**************************************/
int context_set_map(context_t * context, const char * map)
{
	int ret;

	context_lock_list();
	ret = _context_set_map(context,map);
	context_unlock_list();

	return ret;
}

/**************************************
Returns false if error
**************************************/
int context_set_type(context_t * context, const char * type)
{
	int ret = true;

	context_lock_list();
	free( context->type );
	context->type = strdup(type);
	if( context->type == NULL ) {
		ret = false;
	}
	context_unlock_list();

	return ret;
}

/**************************************
**************************************/
void _context_set_pos_tx(context_t * context, unsigned int pos_tx)
{
	context->prev_pos_tx = context->pos_tx;
	if( context->pos_tx != pos_tx ) {
		context->pos_changed = TRUE;
		context->pos_tx = pos_tx;
	}
}

/**************************************
**************************************/
void context_set_pos_tx(context_t * context, unsigned int pos_tx)
{
	context_lock_list();
	_context_set_pos_tx(context,pos_tx);
	context_unlock_list();
}

/**************************************
**************************************/
void _context_set_pos_ty(context_t * context, unsigned int pos_ty)
{
	context->prev_pos_ty = context->pos_ty;
	if( context->pos_ty != pos_ty ) {
		context->pos_changed = TRUE;
		context->pos_ty = pos_ty;
	}
}

/**************************************
**************************************/
void context_set_pos_ty(context_t * context, unsigned int pos_ty)
{
	context_lock_list();
	_context_set_pos_ty(context,pos_ty);
	context_unlock_list();
}

/**************************************
**************************************/
int context_set_id(context_t * context, const char * name)
{
	context_lock_list();

	if( context->id ) {
		free( context->id );
	}
	context->id = strdup(name);
	if( context->id == NULL ) {
		context_unlock_list();
		return false;
	}

	context_unlock_list();
	return true;
}

/**************************************
 * return false if context has already the same values
**************************************/
int context_set_selected_character(context_t * context, const char * id)
{
	context_lock_list();

	if( !strcmp( context->selection.id, id ) ) {
		context_unlock_list();
		return false;
	}
	free( context->selection.id );

	context->selection.id = strdup(id);

	context_unlock_list();
	return true;
}

/**************************************
 * return false if context has already the same values
**************************************/
int context_set_selected_tile(context_t * context, const char * map, int x, int y)
{
	context_lock_list();

	if( !strcmp( context->selection.map, map ) ) {
		if ( x == context->selection.map_coord[0] &&
				y == context->selection.map_coord[1] ) {
			context_unlock_list();
			return false;
		}
	}
	free( context->selection.map );

	context->selection.map = strdup(map);

	context->selection.map_coord[0] = x;
	context->selection.map_coord[1] = y;

	context_unlock_list();
	return true;
}

/**************************************
 * return false if context has already the same values
**************************************/
int context_set_selected_equipment(context_t * context, const char * id)
{
	context_lock_list();

	if( !strcmp( context->selection.equipment, id ) ) {
		context_unlock_list();
		return false;
	}
	free( context->selection.equipment );
	context->selection.equipment = strdup(id);

	context_unlock_list();
	return true;
}

/**************************************
 * return false if context has already the same values
**************************************/
int context_set_selected_item(context_t * context, const char * id)
{
	context_lock_list();

	if( !strcmp( context->selection.inventory, id ) ) {
		context_unlock_list();
		return false;
	}
	free( context->selection.inventory );
	context->selection.inventory = strdup(id);

	context_unlock_list();
	return true;
}

/**************************************
**************************************/
#ifdef SERVER
void register_lua_functions( context_t * context);
#else
/* Client do not use LUA for now */
void register_lua_functions( context_t * context) {}
#endif

/**************************************
**************************************/
void context_new_VM(context_t * context)
{
	context_lock_list();
	context->luaVM = lua_open();
	lua_baselibopen(context->luaVM);
	lua_tablibopen(context->luaVM);
	lua_iolibopen(context->luaVM);
	lua_strlibopen(context->luaVM);
	lua_mathlibopen(context->luaVM);

	register_lua_functions(context);
	context_unlock_list();
}

/*******************************
Update the memory context by reading the character's data file on disk
Return false if there is an error
*******************************/
int context_update_from_file(context_t * context)
{
	/* Don't call context_set_* functions here to avoid inter-blocking */

	char * result;
	int ret  = true;

	context_lock_list();

	if( context->id == NULL ) {
		context_unlock_list();
		return false;
	}

	if(entry_read_string(CHARACTER_TABLE,context->id,&result, CHARACTER_KEY_NAME,NULL)) {
		free( context->character_name );
		context->character_name = result;
	} else {
		ret = false;
	}


	if(entry_read_string(CHARACTER_TABLE,context->id,&result, CHARACTER_KEY_TYPE,NULL)) {
		free( context->type );
		context->type = result;
	} else {
		ret = false;
	}

	if(entry_read_string(CHARACTER_TABLE,context->id,&result, CHARACTER_KEY_MAP,NULL)) {
		free( context->map );
		ret = _context_set_map(context, result);
		free(result);
	} else {
		ret = false;
	}

	int pos_tx;
	if(!entry_read_int(CHARACTER_TABLE,context->id,&pos_tx, CHARACTER_KEY_POS_X,NULL)) {
		ret = false;
	}
	_context_set_pos_tx(context,pos_tx);

	int pos_ty;
	if(!entry_read_int(CHARACTER_TABLE,context->id,&pos_ty, CHARACTER_KEY_POS_Y,NULL)) {
		ret = false;
	}
	_context_set_pos_ty(context,pos_ty);

	context_unlock_list();
	return ret;
}

/*******************************
Write a context to server's disk
*******************************/
int context_write_to_file(context_t * context)
{
	context_lock_list();

	if( context->id == NULL ) {
		context_unlock_list();
		return false;
	}

	entry_write_string(CHARACTER_TABLE, context->id,context->type,CHARACTER_KEY_TYPE,NULL);
	entry_write_string(CHARACTER_TABLE, context->id,context->map,CHARACTER_KEY_MAP, NULL);


	entry_write_int(CHARACTER_TABLE, context->id,context->pos_tx,CHARACTER_KEY_POS_X, NULL);

	entry_write_int(CHARACTER_TABLE, context->id,context->pos_ty,CHARACTER_KEY_POS_Y, NULL);

	context_unlock_list();
	return true;
}

/*******************************
Find a context in memory from its id
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

/**************************************
Spread the data of a context to all in_game context
**************************************/
void context_spread(context_t * context)
{
	context_t * ctx = NULL;

	context_lock_list();

	ctx = context_list_start;

	if( ctx == NULL ) {
		context_unlock_list();
		return;
	}

	do {
		/* Skip if not in game */
		if( ctx->in_game == false ) {
			continue;
		}

		if( context_is_npc(ctx) == true ) {
			continue;
		}

		/* Skip if not on the same map or previous map */
		if( ctx->map && context->map && context->prev_map) {
			if( strcmp(context->map,ctx->map) != 0 &&
					strcmp(context->prev_map,ctx->map) != 0 ) {
				continue;
			}
		}

		network_send_context_to_context(ctx, context);


	} while( (ctx=ctx->next)!= NULL );

	/* The existing context on the previous map should have
	been deleted, we don't need this any more -> this will
	generate less network request */
	free(context->prev_map);
	context->prev_map = NULL;

	context_unlock_list();
}

/**************************************
if "map" == NULL : server sends a message to all connected client
if "map" != NULL : server sends a message to all connected clients on the map
**************************************/
void context_broadcast_text(const char * map, const char * text)
{
	context_t * ctx = NULL;

	context_lock_list();

	ctx = context_list_start;

	if( ctx == NULL ) {
		context_unlock_list();
		return;
	}

	do {
		/* Skip if not in game */
		if( ctx->in_game == false ) {
			continue;
		}

		if( context_is_npc(ctx) == true ) {
			continue;
		}

		/* Skip if the player has not selected its character */
		if( ctx->id == NULL ) {
			continue;
		}

		if( ctx->map == 0 ) {
			continue;
		}

		if(map) {
			/* Skip if not on the same map */
			if( strcmp(map,ctx->map) != 0 ) {
				continue;
			}
		}

		network_send_text(ctx->id, text);

	} while( (ctx=ctx->next)!= NULL );

	context_unlock_list();
}

/**************************************
Send the data of all existing context to the passed context
Useful at start time
**************************************/
void context_request_other_context(context_t * context)
{
	context_t * ctx = NULL;

	context_lock_list();

	ctx = context_list_start;

	if( ctx == NULL ) {
		context_unlock_list();
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

	context_unlock_list();
}

/**************************************
Called from client
**************************************/
void context_add_or_update_from_network_frame(context_t * context,char * data)
{
	context_t * ctx = NULL;
	char * user_name = NULL;
	char * name = NULL;
	char * map = NULL;
	int in_game;
	int connected;
	int pos_tx;
	int pos_ty;
	char * type = NULL;
	char * id = NULL;
	char * selected_character = NULL;
	char * selected_map = NULL;
	int selected_map_x = 0;
	int selected_map_y = 0;
	char * selected_equipment = NULL;
	char * selected_item = NULL;

	/* First decode the data */

	user_name = strdup(data);
	data += (strlen(data)+1);

	name = strdup(data);
	data += (strlen(data)+1);

	map = strdup(data);
	data += (strlen(data)+1);

	in_game = atoi(data);
	data += (strlen(data)+1);

	connected = atoi(data);
	data += (strlen(data)+1);

	pos_tx = atoi(data);
	data += (strlen(data)+1);

	pos_ty = atoi(data);
	data += (strlen(data)+1);

	type = strdup(data);
	data += (strlen(data)+1);

	id = strdup(data);
	data += (strlen(data)+1);

	selected_character = strdup(data);
	data += (strlen(data)+1);

	selected_map = strdup(data);
	data += (strlen(data)+1);

	selected_map_x = atoi(data);
	data += (strlen(data)+1);

	selected_map_y = atoi(data);
	data += (strlen(data)+1);

	selected_equipment = strdup(data);
	data += (strlen(data)+1);

	selected_item = strdup(data);
	data += (strlen(data)+1);

	/* search for this context */
	context_lock_list();
	ctx = context_list_start;

	while( ctx != NULL ) {
		if( strcmp( id, ctx->id) == 0 ) {
			ctx->in_game = in_game;
			ctx->connected = connected;

			if( in_game == true ) {
				wlog(LOGDEBUG,"Updating context %s / %s",user_name,name);
				/* do not call context_set_* function since we already have the lock */
				_context_set_map(ctx,map);

				_context_set_pos_tx(ctx,pos_tx);
				_context_set_pos_ty(ctx,pos_ty);

				free(ctx->type);
				ctx->type = strdup(type);

				if( ctx->selection.map ) {
					free(ctx->selection.map );
				}
				ctx->selection.map = strdup(selected_map);
				ctx->selection.map_coord[0] = selected_map_x;
				ctx->selection.map_coord[1] = selected_map_y;

				if( ctx->selection.id ) {
					free(ctx->selection.id );
				}
				ctx->selection.id = strdup(selected_character);

				if( ctx->selection.equipment ) {
					free(ctx->selection.equipment );
				}
				ctx->selection.equipment = strdup(selected_equipment);

				if( ctx->selection.inventory ) {
					free(ctx->selection.inventory );
				}
				ctx->selection.inventory = strdup(selected_item);
			}

			if( connected == false ) {
				wlog(LOGDEBUG,"Deleting context %s / %s",user_name,name);
				context_free(ctx);
			}
			context_unlock_list();

			goto  context_add_or_update_from_network_frame_free;
		}
		ctx = ctx->next;
	}

	context_unlock_list();

	wlog(LOGDEBUG,"Creating context %s / %s",user_name,name);
	ctx = context_new();
	context_set_username(ctx,user_name);
	context_set_character_name(ctx,name);
	context_set_map(ctx,map);
	context_set_type(ctx,type);
	context_set_pos_tx(ctx,pos_tx);
	context_set_pos_ty(ctx,pos_ty);
	context_set_id(ctx,id);
	context_set_connected(ctx,connected);
	context_set_in_game(ctx,in_game);
	context_set_selected_character(ctx,selected_character);
	context_set_selected_tile(ctx,selected_map,selected_map_x,selected_map_y);
	context_set_selected_equipment(ctx,selected_equipment);
	context_set_selected_item(ctx,selected_item);

context_add_or_update_from_network_frame_free:
	free(user_name);
	free(name);
	free(map);
	free(type);
	free(id);
	free(selected_character);
	free(selected_map);
	free(selected_equipment);
	free(selected_item);
}

/**************************************
Broadcast upload of a map file to all in_game context on that map
**************************************/
void context_broadcast_map(const char * map)
{
	context_t * ctx = NULL;
	char * filename;

	context_lock_list();

	ctx = context_list_start;

	if( ctx == NULL ) {
		context_unlock_list();
		return;
	}

	filename = strconcat(MAP_TABLE,"/",map,NULL);

	do {
		if( context_is_npc(ctx) == true ) {
			continue;
		}

		/* Skip if not in game */
		if( ctx->in_game == false ) {
			continue;
		}

		/* Skip if not on the same map */
		if( ctx->map) {
			if( strcmp(map,ctx->map) != 0 ) {
				continue;
			}
		}

		network_send_file(ctx,filename);

	} while( (ctx=ctx->next)!= NULL );

	free(filename);

	context_unlock_list();
}

/**************************************
Broadcast upload of a character file to all in_game context on the same map
**************************************/
void context_broadcast_character(const char * character)
{
	context_t * ctx = NULL;
	context_t * character_ctx = NULL;
	char * filename;

	context_lock_list();

	character_ctx = context_find(character);

	ctx = context_list_start;

	if( ctx == NULL ) {
		context_unlock_list();
		return;
	}

	filename = strconcat(CHARACTER_TABLE,"/",character,NULL);

	do {
		if( context_is_npc(ctx) == true ) {
			continue;
		}

		/* Skip if not in game */
		if( ctx->in_game == false ) {
			continue;
		}

		/* Skip if not on the same map */
		if( ctx->map) {
			if( strcmp(character_ctx->map,ctx->map) != 0 ) {
				continue;
			}
		}

		network_send_file(ctx,filename);

	} while( (ctx=ctx->next)!= NULL );

	free(filename);

	context_unlock_list();
}
/**************************************
Return the distance between two contexts
**************************************/
int context_distance(context_t * ctx1, context_t * ctx2)
{
	int distx;
	int disty;

	distx = ctx1->pos_tx - ctx2->pos_tx;
	if(distx < 0 ) {
		distx = -distx;
	}
	disty = ctx1->pos_ty - ctx2->pos_ty;
	if(disty < 0 ) {
		disty = -disty;
	}

	return (distx>disty?distx:disty);
}

/**************************************
Reset all contexts position information used for smooth animation
Called on screen switch
**************************************/
void context_reset_all_position()
{
	context_t * ctx = context_get_first();

	context_lock_list();
	while(ctx != NULL ) {
		ctx->move_start_tick = 0;
		ctx = ctx->next;
	}
	context_unlock_list();
}

/**************************************
Return true is context is an NPC
**************************************/
int context_is_npc(context_t * ctx)
{
	if( ctx->socket == NULL && ctx->connected == true) {
		return true;
	}

	return false;
}
