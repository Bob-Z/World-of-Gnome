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
#include <glib/gstdio.h>
#include "../common/common.h"
#include "npc.h"
#include "action.h"

/**********************************
npc_script
*********************************/

static void npc_script(context_t * context, gchar * script, gchar ** parameters)
{
	GTimeVal time;
	gint timeout_ms;

	context_new_VM(context);

	/* Allow the calling thread to continue */
	g_mutex_unlock(context->cond_mutex);

	while(context_get_connected(context)) {
		SDL_LockMutex(npc_mutex);
		timeout_ms = action_execute_script(context,script,parameters);

		g_get_current_time(&time);
		g_time_val_add(&time,timeout_ms * 1000);

		SDL_UnlockMutex(npc_mutex);

		/* The previous call to action_execute_script may have changed
		the connected status. So we test it to avoid waiting for the
		timeout duration before disconnecting */
		if( context_get_connected(context) ) {
			g_cond_timed_wait(context->cond,context->cond_mutex,&time);
		}
	}

	wlog(LOGDEV,"End AI script for %s(%s)",context->id, context->character_name);

	/* Send connected  = FALSE to other context */
	context_spread(context);

	/* clean up */
	context_free(context);
}

/**********************************
manage_npc
*********************************/
static gpointer manage_npc(gpointer data)
{
	context_t * context = (context_t *)data;
	const gchar * ai;
	gchar ** parameters;

	if(read_string(CHARACTER_TABLE,context->id,&ai,CHARACTER_KEY_AI,NULL)) {
		read_list(CHARACTER_TABLE,context->id,&parameters,CHARACTER_KEY_AI_PARAMS,NULL);
		npc_script(context,(gchar *)ai,parameters);
		if(parameters) {
			g_free(parameters);
		}
		return NULL;
	}

	/* Allow the calling thread to continue */
	g_mutex_unlock(context->cond_mutex);
	werr(LOGUSER,"No AI script for %s",context->id);

	return NULL;
}

/***************************
***************************/
void instantiate_npc(const gchar * id)
{
	const gchar * type;
	const gchar * name;
	const gchar * map;
	gint is_npc;
	gint x;
	gint y;
	gint tile_x;
	gint tile_y;
	gint map_x;
	gint map_y;
	context_t * ctx;

	// check if it's a NPC
	if(!read_int(CHARACTER_TABLE,id,&is_npc,CHARACTER_KEY_NPC,NULL)) {
		return;
	}

	if( !is_npc) {
		return;
	}

	// read data of this npc
	if(!read_string(CHARACTER_TABLE,id,&map,CHARACTER_KEY_MAP,NULL)) {
		return;
	}

	if(!read_int(CHARACTER_TABLE,id,&x,CHARACTER_KEY_POS_X,NULL)) {
		return;
	}

	if(!read_int(CHARACTER_TABLE,id,&y,CHARACTER_KEY_POS_Y,NULL)) {
		return;
	}

	if(!read_int(MAP_TABLE,map,&tile_x,MAP_KEY_TILE_SIZE_X,NULL)) {
		return;
	}

	if(!read_int(MAP_TABLE,map,&tile_y,MAP_KEY_TILE_SIZE_Y,NULL)) {
		return;
	}

	if(!read_int(MAP_TABLE,map,&map_x,MAP_KEY_SIZE_X,NULL)) {
		return;
	}

	if(!read_int(MAP_TABLE,map,&map_y,MAP_KEY_SIZE_Y,NULL)) {
		return;
	}

	if(!read_string(CHARACTER_TABLE,id,&name,CHARACTER_KEY_NAME,NULL)) {
		name = "";
	}

	if(!read_string(CHARACTER_TABLE,id,&type,CHARACTER_KEY_TYPE,NULL)) {
		return;
	}

	wlog(LOGDEV,"Creating npc %s of type %s in map %s at %d,%d",name,type,map,x,y);
	ctx = context_new();
	context_set_username(ctx,"CPU");
	context_set_character_name(ctx,name);
	context_set_connected(ctx,1);
	context_set_map(ctx,map);
#if 0
	context_set_map_x(ctx,map_x);
	context_set_map_y(ctx,map_y);
#endif
	context_set_type(ctx,type);
	context_set_pos_x(ctx,x);
	context_set_pos_y(ctx,y);
	context_set_tile_x(ctx,tile_x);
	context_set_tile_y(ctx,tile_y);
	context_set_id(ctx,id);
	ctx->cond = g_cond_new();
	ctx->cond_mutex = g_mutex_new();

	context_spread(ctx);

	/* Make sure the thread has created the LUA VM before continung */
	g_mutex_lock(ctx->cond_mutex);
	/* start management thread */
	g_thread_create(manage_npc,(gpointer)ctx,FALSE,NULL);
	/* Wait for the thread to unlock the mutex */
	g_mutex_lock(ctx->cond_mutex);
	g_mutex_unlock(ctx->cond_mutex);

}
/**************************
init non playing character
***************************/
void init_npc(void)
{
	GDir * dir;
	gchar * dirname;
	const gchar * filename;

	// Read all files in npc directory
	dirname = g_strconcat( g_getenv("HOME"),"/", base_directory, "/", CHARACTER_TABLE,  NULL);
	dir = g_dir_open(dirname,0,NULL);
	while(( filename = g_dir_read_name(dir)) != NULL ) {
		// check for hidden file
		if(filename[0] == '.') {
			continue;
		}

		instantiate_npc(filename);
	}
	g_dir_close(dir);
	g_free(dirname);
}
