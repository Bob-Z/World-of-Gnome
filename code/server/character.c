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
#include <dirent.h>
#include <string.h>
#include "npc.h"
#include "action.h"

/*********************************************
*********************************************/
void character_send_list(context_t * context)
{
	char ** character_list;
	int i = 0;

	if(!read_list(USERS_TABLE, context->user_name,&character_list,USERS_CHARACTER_LIST,NULL)) {
		return;
	}

	while( character_list[i] != NULL ) {
		network_send_command(context, CMD_SEND_CHARACTER, strlen(character_list[i])+1, character_list[i],FALSE);
		i++;
	}

	free(character_list);
}

/*********************************************
*********************************************/
void character_user_send_list(context_t * context)
{
	char * data = NULL;
	Uint32 data_size = 0;
	Uint32 string_size = 0;
	char ** character_list;
	char * type;
	char * name;
	int i;

	if(!read_list(USERS_TABLE, context->user_name,&character_list,USERS_CHARACTER_LIST,NULL)) {
		return;
	}

	i = 0;

	data = strdup("");
	while( character_list[i] != NULL ) {
		if(!entry_read_string(CHARACTER_TABLE, character_list[i], &type, CHARACTER_KEY_TYPE,NULL)) {
			i++;
			continue;
		}

		if(!entry_read_string(CHARACTER_TABLE, character_list[i], &name, CHARACTER_KEY_NAME,NULL)) {
			free(type);
			i++;
			continue;
		}

		/* add the name of the character to the network frame */
		string_size = strlen(character_list[i])+1;
		data = realloc(data, data_size + string_size);
		memcpy(data+data_size,character_list[i], string_size);
		data_size += string_size;

		/* add the type of the character to the network frame */
		string_size = strlen(type)+1;
		data = realloc(data, data_size + string_size);
		memcpy(data+data_size,type, string_size);
		data_size += string_size;

		/* add the type of the character to the network frame */
		string_size = strlen(name)+1;
		data = realloc(data, data_size + string_size);
		memcpy(data+data_size,name, string_size);
		data_size += string_size;

		free(type);
		free(name);
		
		i++;
	}

	free(character_list);

	/* Mark the end of the list */
	data = realloc(data, data_size + 1);
	data[data_size] = 0;
	data_size ++;

	network_send_command(context, CMD_SEND_USER_CHARACTER, data_size, data,FALSE);
	free(data);
}

/*****************************/
/* disconnect a character */
/* return -1 if fails */
int character_disconnect( const char * id)
{
	context_t * ctx;

	ctx = context_find(id);
	/* For NPC */
	if( ctx->output_stream == NULL ) {
		context_set_connected(ctx,FALSE);
		/* Wake up NPC */
		if( SDL_TryLockMutex (ctx->cond_mutex) == TRUE ) {
			SDL_CondSignal (ctx->cond);
			SDL_UnlockMutex (ctx->cond_mutex);
		}
	}
	/* For player */
	/* TODO */
	return 0;
}

/******************************************************
Create a new character based on the specified template
return the id of the newly created character
the returned string MUST BE FREED by caller
return NULL if fails
*******************************************************/
char * character_create_from_template(context_t * ctx,const char * template,const char * map, int x, int y)
{
	char * new_id;
	char * templatename;
	char * fullname;
	char * filename;

	new_id = file_new(CHARACTER_TABLE);

	templatename = strconcat(getenv("HOME"),"/",base_directory,"/",CHARACTER_TEMPLATE_TABLE,"/",template,NULL);

	filename = strconcat(CHARACTER_TABLE,"/",new_id,NULL);

	fullname = strconcat(getenv("HOME"),"/",base_directory,"/",filename,NULL);

	file_copy(templatename,fullname);
	free(templatename);
	free(fullname);

	/* Check if new character is allowed to be created here */
	if(!map_check_tile(ctx,new_id,map,x,y)) {
		entry_destroy(filename);
		free(filename);
		free(new_id);
		return NULL;
	}

	/* Write position */
	if(!write_string(CHARACTER_TABLE,new_id,map,CHARACTER_KEY_MAP,NULL)) {
		entry_destroy(filename);
		free(filename);
		free(new_id);
		return NULL;
	}

	if(!write_int(CHARACTER_TABLE,new_id,x,CHARACTER_KEY_POS_X,NULL)) {
		entry_destroy(filename);
		free(filename);
		free(new_id);
		return NULL;
	}

	if(!write_int(CHARACTER_TABLE,new_id,y,CHARACTER_KEY_POS_Y,NULL)) {
		entry_destroy(filename);
		free(filename);
		free(new_id);
		return NULL;
	}

	free(filename);
	
	return new_id;
}

/***********************************************************************
Call aggro script for each context in every npc context aggro dist
***********************************************************************/
void character_update_aggro(context_t * context)
{
	context_t * ctx = NULL;
	context_t * ctx2 = NULL;
	int aggro_dist;
	int dist;
	char * aggro_script;
	char * param[] = { NULL,NULL };
	int no_aggro = 1;

	ctx = context_get_list_first();

	if( ctx == NULL ) {
		return;
	}

	if( ctx->map == NULL ) {
		return;
	}

	/*For each context: check dist of every other context on the same map*/
	/*For each context at aggro distance, execute the accro script */
	do {
		if(!ctx->id) {
			continue;
		}

		if( ctx->luaVM == NULL ) {
			continue;
		}

		no_aggro = 1;
		if(!read_int(CHARACTER_TABLE,ctx->id,&aggro_dist, CHARACTER_KEY_AGGRO_DIST,NULL)) {
			continue;
		}

		if(!entry_read_string(CHARACTER_TABLE,ctx->id,&aggro_script, CHARACTER_KEY_AGGRO_SCRIPT,NULL)) {
			continue;
		}

		ctx2 = context_get_list_first();

		do {
			/* Skip current context */
			if( ctx == ctx2) {
				continue;
			}

			if(ctx2->id == NULL) {
				continue;
			}

			if(ctx2->map == NULL) {
				continue;
			}

			/* Skip if not on the same map */
			if( strcmp(ctx->map,ctx2->map) != 0 ) {
				continue;
			}

			dist = context_distance(ctx,ctx2);

			if(dist <= aggro_dist) {
				param[0] = ctx2->id;
				param[1] = NULL;
				action_execute_script(ctx,aggro_script,param);
				no_aggro = 0;
			}
		} while( (ctx2=ctx2->next)!= NULL );


		/* Notify if no aggro available */
		if( no_aggro ) {
			param[0] = "";
			param[1] = NULL;
			action_execute_script(ctx,aggro_script,param);
		}
		
		free(aggro_script);
		
	} while( (ctx=ctx->next)!= NULL );
}

/******************************************************
return 0 if new position OK or if position has not changed.
return -1 if the position was not set (because tile not allowed or out of bound)
******************************************************/
int character_set_pos(context_t * ctx, char * map, int x, int y)
{
	char ** event_id;
	char * script;
	char ** param;
	int i;
	int change_map = 0;

	if(ctx == NULL) {
		return -1;
	}

	/* Do nothing if no move */
	if(!strcmp(ctx->map, map) && ctx->pos_x == x && ctx->pos_y == y) {
		return 0;
	}

	/* Check if this character is allowed to go to the target tile */
	if (map_check_tile(ctx,ctx->id,map,x,y) ) {

		if( strcmp(ctx->map,map) ) {
			change_map = 1;
		}

		context_set_map(ctx,map);
		context_set_pos_x(ctx,x);
		context_set_pos_y(ctx,y);

		write_string(CHARACTER_TABLE,ctx->id,map,CHARACTER_KEY_MAP,NULL);
		write_int(CHARACTER_TABLE,ctx->id,x,CHARACTER_KEY_POS_X,NULL);
		write_int(CHARACTER_TABLE,ctx->id,y,CHARACTER_KEY_POS_Y,NULL);

		context_spread(ctx);
		if(change_map) {
			context_request_other_context(ctx);
		}

		event_id = map_get_event(map,x,y);

		if(event_id) {
			i = 0;
			while(event_id[i]) {
				param=NULL;
				if( !entry_read_string(MAP_TABLE,map,&script,MAP_ENTRY_EVENT_LIST,event_id[i],MAP_EVENT_SCRIPT,NULL) ) {
					i++;
					continue;
				}
				read_list(MAP_TABLE,map,&param,MAP_ENTRY_EVENT_LIST,event_id[i],MAP_EVENT_PARAM,NULL);

				action_execute_script(ctx,script,param);
				free(script);

				i++;
				free(param);
			}
			free(event_id);
		}

		character_update_aggro(ctx);
		return 0;
	}
	return -1;
}

/*********************************************************
 Set NPC to the value passed.
 If the value is != 0 , the NPC is instanciated
 return -1 on error
*********************************************************/
int character_set_npc(const char * id, int npc)
{
	if(!write_int(CHARACTER_TABLE,id,npc,CHARACTER_KEY_NPC,NULL)) {
		return -1;
	}

	if(npc) {
		instantiate_npc(id);
	}

	return 0;
}
