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

#ifndef CONTEXT_H
#define CONTEXT_H

#include "common.h"
#include <string.h>
#include <glib.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <glib/gstdio.h>

context_t * context_list_start = NULL;
GStaticMutex context_list_mutex = G_STATIC_MUTEX_INIT; /* exclusive access to context_list */

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

	context->sprite_image = NULL;

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

	g_static_mutex_lock(&context_list_mutex);
	if ( context_list_start == NULL ) {
		context_list_start = g_new0(context_t,1);
		context_init(context_list_start);
		g_static_mutex_unlock(&context_list_mutex);
		return context_list_start;
	}

	ctx = context_list_start;
	while( ctx->next != NULL ) {
		ctx = ctx->next;
	}

	ctx->next = g_new0(context_t,1);
	context_init(ctx->next);
	ctx->next->previous = ctx;
	g_static_mutex_unlock(&context_list_mutex);
	return ctx->next;
}

/* context_free
  Deep free of a context_t struct
*/
void context_free(context_t * context)
{
	gchar * filename;
	gint delete_file = TRUE;
	context_t * ctx;

	g_static_mutex_lock (&context_list_mutex);

	filename = g_strconcat( g_getenv("HOME"),"/", base_directory, "/", CHARACTER_TABLE, "/", context->id, NULL);
// We should not errase the file's data from memory until it's dumped to disk, so comment this line
//	entry_destroy(context->id);


	g_free(context->user_name);
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
	g_free(context->hostname);
	context->hostname = NULL;
	context->send_thread = NULL;
	g_free(context->character_name);
	context->character_name = NULL;
	g_free(context->map);
	context->map = NULL;
	g_free(context->type);
	context->type = NULL;
	if( context->sprite_image != NULL ) {
		gtk_widget_destroy(context->sprite_image);
		context->sprite_image = NULL;
	}
	context->selection.id = NULL;
	context->selection.map_coord[0] = -1;
	context->selection.map_coord[1] = -1;
	context->selection.map = NULL;
	g_free(context->selection.inventory);
	context->selection.inventory = NULL;
	g_free(context->selection.equipment);
	context->selection.equipment = NULL;
	g_free(context->id);
	context->id = NULL;
	g_free(context->prev_map);
	context->prev_map = NULL;
	if( context->luaVM != NULL) {
		lua_close(context->luaVM);
	}
	if( context->cond != NULL) {
		g_cond_free(context->cond);
	}
	if( context->cond_mutex != NULL) {
		g_mutex_free(context->cond_mutex);
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
		if (g_strcmp0(context->id,ctx->selection.id)==0) {
			ctx->selection.id = NULL;
		}
		ctx = ctx->next;
	}

	g_free(context);

	g_static_mutex_unlock (&context_list_mutex);

	if(delete_file) {
		g_unlink(filename);
	}
	g_free(filename);
}

void context_lock_list()
{
	g_static_mutex_lock (&context_list_mutex);
}

void context_unlock_list()
{
	g_static_mutex_unlock (&context_list_mutex);
}

context_t * context_get_list_first()
{
	return context_list_start;
}

/* context_set_username
  Set user_name

  Returns FALSE if error
*/
gboolean context_set_username(context_t * context, const gchar * name)
{
	g_static_mutex_lock (&context_list_mutex);

	g_free( context->user_name );
	context->user_name = g_strdup(name);
	if( context->user_name == NULL ) {
		g_static_mutex_unlock (&context_list_mutex);
		return FALSE;
	}

	g_static_mutex_unlock (&context_list_mutex);
	return TRUE;
}

/* context_set_connected
  Set connected flag
*/
void context_set_connected(context_t * context, gboolean connected)
{
	g_static_mutex_lock (&context_list_mutex);
	context->connected = connected;
	g_static_mutex_unlock (&context_list_mutex);
}
/* context_get_connected
  Get connection flag
*/
gboolean context_get_connected(context_t * context)
{
	gboolean conn = FALSE;

	g_static_mutex_lock (&context_list_mutex);
	conn = context->connected;
	g_static_mutex_unlock (&context_list_mutex);

	return conn;
}

void context_set_input_stream(context_t * context, GInputStream * stream)
{
	g_static_mutex_lock (&context_list_mutex);
	context->input_stream = stream;
	g_static_mutex_unlock (&context_list_mutex);
}

GInputStream * context_get_input_stream(context_t * context)
{
	GInputStream * conn = NULL;

	g_static_mutex_lock (&context_list_mutex);
	conn = context->input_stream;
	g_static_mutex_unlock (&context_list_mutex);

	return conn;
}

void context_set_output_stream(context_t * context, GOutputStream * stream)
{
	g_static_mutex_lock (&context_list_mutex);
	context->output_stream = stream;
	g_static_mutex_unlock (&context_list_mutex);
}

GOutputStream * context_get_output_stream(context_t * context)
{
	GOutputStream * conn = NULL;

//FIXME
//	g_static_mutex_lock (&context_list_mutex);
	conn = context->output_stream;
//	g_static_mutex_unlock (&context_list_mutex);

	return conn;
}

void context_set_connection(context_t * context, GSocketConnection * connection)
{
	g_static_mutex_lock (&context_list_mutex);
	context->connection = connection;
	g_static_mutex_unlock (&context_list_mutex);
}

GSocketConnection * context_get_connection(context_t * context)
{
	GSocketConnection * conn = NULL;

	g_static_mutex_lock (&context_list_mutex);
	conn = context->connection;
	g_static_mutex_unlock (&context_list_mutex);

	return conn;
}
/* context_set_character_name
  Set name

  Returns FALSE if error
*/
gboolean context_set_character_name(context_t * context, const gchar * name)
{
	int ret = TRUE;

	g_static_mutex_lock (&context_list_mutex);
	g_free( context->character_name );
	context->character_name = g_strdup(name);
	if( context->character_name == NULL ) {
		ret = FALSE;
	}
	g_static_mutex_unlock (&context_list_mutex);

	return ret;
}
/* context_set_map
  Set map name

  Returns FALSE if error
*/
static gboolean _context_set_map(context_t * context, const gchar * map)
{
	gint map_x;
	gint map_y;

	if(context->prev_map != NULL) {
		if(!strcmp(context->map,map)) {
			return TRUE;
		}
	}

	if(!read_int(MAP_TABLE,map,&map_x,MAP_KEY_SIZE_X,NULL)) {
		return FALSE;
	}
	if(!read_int(MAP_TABLE,map,&map_y,MAP_KEY_SIZE_Y,NULL)) {
		return FALSE;
	}

	g_free( context->prev_map );
	context->prev_map = g_strdup(context->map);
	g_free( context->map );
	context->map = g_strdup(map);
	if( context->map == NULL ) {
		return FALSE;
	}
	context->map_x = map_x;
	context->map_y = map_y;
	context->change_map = 1;

	return TRUE;
}
gboolean context_set_map(context_t * context, const gchar * map)
{
	int ret;

	g_static_mutex_lock (&context_list_mutex);
	ret = _context_set_map(context,map);
	g_static_mutex_unlock (&context_list_mutex);

	return ret;
}
/* context_set_type
  Set type name

  Returns FALSE if error
*/
gboolean context_set_type(context_t * context, const gchar * type)
{
	int ret = TRUE;

	g_static_mutex_lock (&context_list_mutex);
	g_free( context->type );
	context->type = g_strdup(type);
	if( context->type == NULL ) {
		ret = FALSE;
	}
	g_static_mutex_unlock (&context_list_mutex);

	return ret;
}

/* context_set_pos_x
  Set pos_x
*/
void context_set_pos_x(context_t * context, guint pos)
{
	g_static_mutex_lock (&context_list_mutex);
	context->pos_x = pos;
	g_static_mutex_unlock (&context_list_mutex);
}

/* context_set_pos_y
  Set pos_y
*/
void context_set_pos_y(context_t * context, guint pos)
{
	g_static_mutex_lock (&context_list_mutex);
	context->pos_y = pos;
	g_static_mutex_unlock (&context_list_mutex);
}

/* context_set_tile_x
  Set tile_x
*/
void context_set_tile_x(context_t * context, guint pos)
{
	g_static_mutex_lock (&context_list_mutex);
	context->tile_x = pos;
	g_static_mutex_unlock (&context_list_mutex);
}

/* context_set_tile_y
  Set tile_y
*/
void context_set_tile_y(context_t * context, guint pos)
{
	g_static_mutex_lock (&context_list_mutex);
	context->tile_y = pos;
	g_static_mutex_unlock (&context_list_mutex);
}

/* context_set_id
  Set id

  Returns FALSE if error
*/
gboolean context_set_id(context_t * context, const gchar * name)
{
	g_static_mutex_lock (&context_list_mutex);

	g_free( context->id );
	context->id = g_strdup(name);
	if( context->id == NULL ) {
		g_static_mutex_unlock (&context_list_mutex);
		return FALSE;
	}

	g_static_mutex_unlock (&context_list_mutex);
	return TRUE;
}

void register_lua_functions( context_t * context);
void context_new_VM(context_t * context)
{
	g_static_mutex_lock (&context_list_mutex);
	context->luaVM = lua_open();
	lua_baselibopen(context->luaVM);
	lua_tablibopen(context->luaVM);
	lua_iolibopen(context->luaVM);
	lua_strlibopen(context->luaVM);
	lua_mathlibopen(context->luaVM);

	register_lua_functions(context);
	g_static_mutex_unlock (&context_list_mutex);
}
/*******************************
Update the memory context by reading the client's character data file on disk
*******************************/
gboolean context_update_from_file(context_t * context)
{
	/* Don't call context_set_* functions here to avoid inter-blocking */

	const gchar * result;
	int ret;

	g_static_mutex_lock (&context_list_mutex);

	if( context->id == NULL ) {
		g_static_mutex_unlock (&context_list_mutex);
		return FALSE;
	}

	if(!read_string(CHARACTER_TABLE,context->id,&result, CHARACTER_KEY_NAME,NULL)) {
		g_static_mutex_unlock (&context_list_mutex);
		return FALSE;
	}

	g_free( context->character_name );
	context->character_name = g_strdup(result);
	if( context->character_name == NULL ) {
		g_static_mutex_unlock (&context_list_mutex);
		return FALSE;
	}

	if(!read_string(CHARACTER_TABLE,context->id,&result, CHARACTER_KEY_TYPE,NULL)) {
		g_static_mutex_unlock (&context_list_mutex);
		return FALSE;
	}

	g_free( context->type );
	context->type = g_strdup(result);
	if( context->type == NULL ) {
		g_static_mutex_unlock (&context_list_mutex);
		return FALSE;
	}

	if(!read_string(CHARACTER_TABLE,context->id,&result, CHARACTER_KEY_MAP,NULL)) {
		g_static_mutex_unlock (&context_list_mutex);
		return FALSE;
	}

	g_free( context->map );
	ret = _context_set_map(context, result);
	if( ret == FALSE ) {
		g_static_mutex_unlock (&context_list_mutex);
		return FALSE;
	}

	if(!read_int(CHARACTER_TABLE,context->id,&context->pos_x, CHARACTER_KEY_POS_X,NULL)) {
		g_static_mutex_unlock (&context_list_mutex);
		return FALSE;
	}

	if(!read_int(CHARACTER_TABLE,context->id,&context->pos_y, CHARACTER_KEY_POS_Y,NULL)) {
		g_static_mutex_unlock (&context_list_mutex);
		return FALSE;
	}

	g_static_mutex_unlock (&context_list_mutex);
	return TRUE;
}

/*******************************
Write a context to server's disk
*******************************/
gboolean context_write_to_file(context_t * context)
{
	g_static_mutex_lock (&context_list_mutex);

	if( context->id == NULL ) {
		g_static_mutex_unlock (&context_list_mutex);
		return FALSE;
	}

	write_string(CHARACTER_TABLE, context->id,context->type,CHARACTER_KEY_TYPE,NULL);
	write_string(CHARACTER_TABLE, context->id,context->map,CHARACTER_KEY_MAP, NULL);


	write_int(CHARACTER_TABLE, context->id,context->pos_x,CHARACTER_KEY_POS_X, NULL);

	write_int(CHARACTER_TABLE, context->id,context->pos_y,CHARACTER_KEY_POS_Y, NULL);

	g_static_mutex_unlock (&context_list_mutex);
	return TRUE;
}

/*******************************
Find a context in memroy from its id
*******************************/
context_t * context_find(const gchar * id)
{
	context_t * ctx;

	ctx = context_list_start;

	while(ctx != NULL ) {
		if( g_strcmp0(ctx->id,id) == 0 ) {
			return ctx;
		}

		ctx = ctx->next;
	}

	return NULL;
}

/*******************************
Update the context by decoding the received frame
Called by server
*******************************/
gboolean context_update_from_network_frame(context_t * context, gchar * frame)
{
	gchar * data = frame;

	g_static_mutex_lock (&context_list_mutex);

	if( context->user_name ) {
		g_free( context->user_name );
	}
	context->user_name = g_strdup(data);

	data += (g_utf8_strlen(data,-1)+1);

	if( context->character_name ) {
		g_free( context->character_name );
	}
	context->character_name = g_strdup(data);
	data += (g_utf8_strlen(data,-1)+1);

	_context_set_map(context,data);
	data += (g_utf8_strlen(data,-1)+1);

	context->connected = g_ascii_strtoll(data,NULL,10);
	data += (g_utf8_strlen(data,-1)+1);

	context->pos_x = g_ascii_strtoll(data,NULL,10);
	data += (g_utf8_strlen(data,-1)+1);

	context->pos_y = g_ascii_strtoll(data,NULL,10);
	data += (g_utf8_strlen(data,-1)+1);

	if( context->type ) {
		g_free( context->type );
	}
	context->type = g_strdup(data);
	data += (g_utf8_strlen(data,-1)+1);

	context->tile_x = g_ascii_strtoll(data,NULL,10);
	data += (g_utf8_strlen(data,-1)+1);

	context->tile_y = g_ascii_strtoll(data,NULL,10);
	data += (g_utf8_strlen(data,-1)+1);

	if( context->id ) {
		g_free( context->id );
	}
	context->id = g_strdup(data);
	data += (g_utf8_strlen(data,-1)+1);

	if( context->selection.id ) {
		g_free( context->selection.id );
	}
	if( data[0] != 0 ) {
		context->selection.id = g_strdup(data);
	} else {
		context->selection.id = NULL;
	}
	data += (g_utf8_strlen(data,-1)+1);

	context->selection.map_coord[0] = g_ascii_strtoll(data,NULL,10);
	data += (g_utf8_strlen(data,-1)+1);

	context->selection.map_coord[1] = g_ascii_strtoll(data,NULL,10);
	data += (g_utf8_strlen(data,-1)+1);

	if( context->selection.map ) {
		g_free( context->selection.map );
	}
	if( data[0] != 0 ) {
		context->selection.map = g_strdup(data);
	} else {
		context->selection.map = NULL;
	}
	data += (g_utf8_strlen(data,-1)+1);

	if( context->selection.inventory ) {
		g_free( context->selection.inventory );
	}
	if( data[0] != 0 ) {
		context->selection.inventory = g_strdup(data);
	} else {
		context->selection.inventory = NULL;
	}
	data += (g_utf8_strlen(data,-1)+1);

	if( context->selection.equipment ) {
		g_free( context->selection.equipment );
	}
	if( data[0] != 0 ) {
		context->selection.equipment = g_strdup(data);
	} else {
		context->selection.equipment = NULL;
	}
	data += (g_utf8_strlen(data,-1)+1);

	g_static_mutex_unlock (&context_list_mutex);

	return TRUE;
}

/* Spread the data of a context to all connected context */
void context_spread(context_t * context)
{
	context_t * ctx = NULL;

	g_static_mutex_lock (&context_list_mutex);

	ctx = context_list_start;

	if( ctx == NULL ) {
		g_static_mutex_unlock (&context_list_mutex);
		return;
	}

	do {
		/* Skip NPC */
		if( ctx->output_stream == NULL ) {
			continue;
		}

		/* Skip if not on the same map or previous map */
		if( g_strcmp0(context->map,ctx->map) != 0 &&
				g_strcmp0(context->prev_map,ctx->map) != 0 ) {
			continue;
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
	g_free(context->prev_map);
	context->prev_map = NULL;

	g_static_mutex_unlock (&context_list_mutex);
}

/*if "map" == NULL : server sends a message to all connected client
 if "map" != NULL : server sends a message to all connected clients on the map
*/
void context_broadcast_text(const gchar * map, const gchar * text)
{
	context_t * ctx = NULL;

	g_static_mutex_lock (&context_list_mutex);

	ctx = context_list_start;

	if( ctx == NULL ) {
		g_static_mutex_unlock (&context_list_mutex);
		return;
	}

	do {
		/* Skip NPC */
		if( ctx->output_stream == NULL ) {
			continue;
		}

		if(map) {
			/* Skip if not on the same map */
			if( g_strcmp0(map,ctx->map) != 0 ) {
				continue;
			}

			/* Skip if the player has not selected its character */
			if( ctx->id == NULL ) {
				continue;
			}
		}

		network_send_text(ctx->id, text);

	} while( (ctx=ctx->next)!= NULL );

	g_static_mutex_unlock (&context_list_mutex);
}

/* Send the data of all existing context to the passed context */
/* Useful at start time */
void context_request_other_context(context_t * context)
{
	context_t * ctx = NULL;

	g_static_mutex_lock (&context_list_mutex);

	ctx = context_list_start;

	if( ctx == NULL ) {
		g_static_mutex_unlock (&context_list_mutex);
		return;
	}

	do {
		/* Skip the calling context */
		if( context == ctx ) {
			continue;
		}

		/* Skip if not on the same map */
		if( g_strcmp0(context->map,ctx->map) != 0 ) {
			continue;
		}

		network_send_context_to_context(context, ctx);

	} while( (ctx=ctx->next)!= NULL );

	g_static_mutex_unlock (&context_list_mutex);
}

/* Called from client */
void context_add_or_update_from_network_frame(context_t * context,gchar * data)
{
	context_t * ctx = NULL;
	gchar * user_name = NULL;
	gchar * name = NULL;
	gchar * map = NULL;
	gboolean connected;
	gint pos_x;
	gint pos_y;
	gint tile_x;
	gint tile_y;
	gchar * type = NULL;
	gchar * id = NULL;

	/* First decode the data */

	user_name = g_strdup(data);
	data += (g_utf8_strlen(data,-1)+1);

	name = g_strdup(data);
	data += (g_utf8_strlen(data,-1)+1);

	map = g_strdup(data);
	data += (g_utf8_strlen(data,-1)+1);

	connected = g_ascii_strtoll(data,NULL,10);
	data += (g_utf8_strlen(data,-1)+1);

	pos_x = g_ascii_strtoll(data,NULL,10);
	data += (g_utf8_strlen(data,-1)+1);

	pos_y = g_ascii_strtoll(data,NULL,10);
	data += (g_utf8_strlen(data,-1)+1);

	type = g_strdup(data);
	data += (g_utf8_strlen(data,-1)+1);

	tile_x = g_ascii_strtoll(data,NULL,10);
	data += (g_utf8_strlen(data,-1)+1);

	tile_y = g_ascii_strtoll(data,NULL,10);
	data += (g_utf8_strlen(data,-1)+1);

	id = g_strdup(data);
	data += (g_utf8_strlen(data,-1)+1);

	/* search for this context */
	g_static_mutex_lock (&context_list_mutex);
	ctx = context_list_start;

	while( ctx != NULL ) {
		if( g_strcmp0( id, ctx->id) == 0 ) {
			if( connected != FALSE ) {
				wlog(LOGDEBUG,"Updating context %s / %s",user_name,name);
				/* do not call context_set_* function since we already have the lock */
				_context_set_map(ctx,map);

				ctx->pos_x = pos_x;
				ctx->pos_y = pos_y;

				ctx->tile_x = tile_x;
				ctx->tile_y = tile_y;

				g_free(ctx->type);
				ctx->type = type;

				g_free(user_name);
				g_free(name);
				g_free(id);

				g_static_mutex_unlock (&context_list_mutex);
			} else {
				wlog(LOGDEBUG,"Deleting context %s / %s",user_name,name);
				/* Delete selection if it was selected */
				if( context->selection.id != NULL ) {
					if( g_strcmp0(context->selection.id, id) == 0 ) {
						context->selection.id = NULL;
						network_send_context(context);
					}

				}
				g_static_mutex_unlock (&context_list_mutex);
				context_free(ctx);
			}
			return;
		}
		ctx = ctx->next;
	}

	g_static_mutex_unlock (&context_list_mutex);

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

	g_free(user_name);
	g_free(name);
	g_free(map);
	g_free(type);
}

/* Broadcast upload of a file to all connected context */
void context_broadcast_file(const gchar * table, const gchar * file, gboolean same_map_only)
{
	context_t * ctx = NULL;
	gchar * filename;

	g_static_mutex_lock (&context_list_mutex);

	ctx = context_list_start;

	if( ctx == NULL ) {
		g_static_mutex_unlock (&context_list_mutex);
		return;
	}

	filename = g_strconcat( table , "/", file, NULL);

	do {
		/* Skip if not connected (NPC) */
		if( ctx->output_stream == NULL ) {
			continue;
		}
		/* Skip if not on the same map */
		if( same_map_only ) {
			if( g_strcmp0(file,ctx->map) != 0 ) {
				continue;
			}
		}

		network_send_file(ctx,filename);

	} while( (ctx=ctx->next)!= NULL );

	g_static_mutex_unlock (&context_list_mutex);
}

/* Return the distance between two contexts */
gint context_distance(context_t * ctx1, context_t * ctx2)
{
	gint distx;
	gint disty;

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
#endif
