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

#include "../common/common.h"
#include "npc.h"
#include "action.h"
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

#define NPC_TIMEOUT	(2000)

/**********************************
npc_script
*********************************/
static void npc_script(context_t * context, char * script, char ** parameters)
{
	Uint32 timeout_ms;

	/* Do not start every NPC at the same moment */
	usleep( (random()%NPC_TIMEOUT) * 1000);

	context_new_VM(context);

	wlog(LOGDEV,"Start AI script for %s(%s)",context->id, context->character_name);

	while(context_get_connected(context)) {
		SDL_LockMutex(npc_mutex);
		timeout_ms = action_execute_script(context,script,parameters);

		SDL_UnlockMutex(npc_mutex);

		/* The previous call to action_execute_script may have changed
		the connected status. So we test it to avoid waiting for the
		timeout duration before disconnecting */
		if( context_get_connected(context) ) {
			SDL_LockMutex(context->cond_mutex);
			SDL_CondWaitTimeout(context->cond,context->cond_mutex,timeout_ms);
			SDL_UnlockMutex(context->cond_mutex);
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
static int manage_npc(void * data)
{
	context_t * context = (context_t *)data;
	char * ai;
	char ** parameters;

	if(entry_read_string(CHARACTER_TABLE,context->id,&ai,CHARACTER_KEY_AI,NULL)) {
		entry_read_list(CHARACTER_TABLE,context->id,&parameters,CHARACTER_KEY_AI_PARAMS,NULL);
		npc_script(context,ai,parameters);
		free(ai);
		deep_free(parameters);
		return 0;
	}

	werr(LOGUSER,"No AI script for %s",context->id);

	return 0;
}

/***************************
***************************/
void instantiate_npc(const char * id)
{
	char * type;
	char * name;
	char * map;
	int is_npc;
	int x;
	int y;
	int tile_w;
	int tile_h;
	int map_w;
	int map_h;
	context_t * ctx;
	char buf[512];

	// check if it's a NPC
	if(!entry_read_int(CHARACTER_TABLE,id,&is_npc,CHARACTER_KEY_NPC,NULL)) {
		return;
	}

	if( !is_npc) {
		return;
	}

	// read data of this npc
	if(!entry_read_int(CHARACTER_TABLE,id,&x,CHARACTER_KEY_POS_X,NULL)) {
		return;
	}

	if(!entry_read_int(CHARACTER_TABLE,id,&y,CHARACTER_KEY_POS_Y,NULL)) {
		return;
	}

	if(!entry_read_string(CHARACTER_TABLE,id,&map,CHARACTER_KEY_MAP,NULL)) {
		return;
	}

	if(!entry_read_int(MAP_TABLE,map,&tile_w,MAP_KEY_TILE_WIDTH,NULL)) {
		free(map);
		return;
	}

	if(!entry_read_int(MAP_TABLE,map,&tile_h,MAP_KEY_TILE_HEIGHT,NULL)) {
		free(map);
		return;
	}

	if(!entry_read_int(MAP_TABLE,map,&map_w,MAP_KEY_WIDTH,NULL)) {
		free(map);
		return;
	}

	if(!entry_read_int(MAP_TABLE,map,&map_h,MAP_KEY_HEIGHT,NULL)) {
		free(map);
		return;
	}

	if(!entry_read_string(CHARACTER_TABLE,id,&name,CHARACTER_KEY_NAME,NULL)) {
		name = strdup("");
	}

	if(!entry_read_string(CHARACTER_TABLE,id,&type,CHARACTER_KEY_TYPE,NULL)) {
		free(map);
		free(name);
		return;
	}

	wlog(LOGDEV,"Creating npc %s of type %s in map %s at %d,%d",name,type,map,x,y);
	ctx = context_new();
	context_set_username(ctx,"CPU");
	context_set_character_name(ctx,name);
	free(name);
	context_set_in_game(ctx,true);
	context_set_connected(ctx,true);
	context_set_map(ctx,map);
	free(map);
#if 0
	context_set_map_w(ctx,map_w);
	context_set_map_h(ctx,map_h);
#endif
	context_set_type(ctx,type);
	free(type);
	context_set_pos_x(ctx,x);
	context_set_pos_y(ctx,y);
	context_set_tile_x(ctx,tile_w);
	context_set_tile_y(ctx,tile_h);
	context_set_id(ctx,id);
	ctx->cond = SDL_CreateCond();
	ctx->cond_mutex = SDL_CreateMutex();

	context_spread(ctx);

	sprintf(buf,"npc:%s",id);
	SDL_CreateThread(manage_npc,buf,(void*)ctx);
}
/**************************
init non playing character
***************************/
void init_npc(void)
{
	DIR * dir;
	char * dirname;
	struct dirent * ent;

	// Read all files in npc directory
	dirname = strconcat(base_directory,"/",CHARACTER_TABLE,NULL);

	dir = opendir(dirname);
	free(dirname);
	while(( ent = readdir(dir)) != NULL ) {
		// skip hidden file
		if(ent->d_name[0] == '.') {
			continue;
		}

		instantiate_npc(ent->d_name);
	}
	closedir(dir);
}

