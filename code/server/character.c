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
#include "character.h"

/*********************************************
Send playable character templates
*********************************************/
void character_send_list(context_t * context)
{
//TODO
#if 0
	char ** character_list;
	int i = 0;

	if(!entry_read_list(USERS_TABLE, context->user_name,&character_list,USERS_CHARACTER_LIST,NULL)) {
		return;
	}

	while( character_list[i] != NULL ) {
		network_send_command(context, CMD_SEND_CHARACTER, strlen(character_list[i])+1, character_list[i],FALSE);
		i++;
	}

	deep_free(character_list);
#endif
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

	if(!entry_read_list(USERS_TABLE, context->user_name,&character_list,USERS_CHARACTER_LIST,NULL)) {
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

	deep_free(character_list);

	/* Mark the end of the list */
	data = realloc(data, data_size + 1);
	data[data_size] = 0;
	data_size ++;

	network_send_command(context, CMD_SEND_USER_CHARACTER, data_size, data,FALSE);
	free(data);
}

/*****************************
 Disconnect a character.
 This kill a NPC AI thread
return -1 if fails
*****************************/
int character_disconnect( const char * id)
{
	context_t * ctx;

	werr(LOGDEBUG,"Disconnecting %s",id);

	ctx = context_find(id);
	context_set_in_game(ctx,false);
	context_set_connected(ctx,false);
	context_spread(ctx);

	if( context_is_npc(ctx) == true ) {
		/* Wake up NPC */
		if( SDL_TryLockMutex (ctx->cond_mutex) == 0 ) {
			SDL_CondSignal (ctx->cond);
			SDL_UnlockMutex (ctx->cond_mutex);
		}
	}

	return 0;
}
/*****************************
 Kick a character out of the game
 It does not disconnect it.
 An NPC could re pop from an out of game state.
 A player can go back in game after choosing a new character id
return -1 if fails
*****************************/
int character_out_of_game( const char * id)
{
	context_t * ctx;

	werr(LOGDEBUG,"Kicking %s out of the game",id);

	ctx = context_find(id);
	context_set_in_game(ctx,false);
	context_spread(ctx);

	if( context_is_npc(ctx) == true ) {
		/* Wake up NPC */
		if( SDL_TryLockMutex (ctx->cond_mutex) == 0 ) {
			SDL_CondSignal (ctx->cond);
			SDL_UnlockMutex (ctx->cond_mutex);
		}
	}

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

	new_id = file_new(CHARACTER_TABLE);

	templatename = strconcat(base_directory,"/",CHARACTER_TEMPLATE_TABLE,"/",template,NULL);
	fullname = strconcat(base_directory,"/",CHARACTER_TABLE,"/",new_id,NULL);
	file_copy(templatename,fullname);
	free(templatename);
	free(fullname);

	/* Check if new character is allowed to be created here */
	if(!map_check_tile(ctx,new_id,map,x,y)) {
		entry_destroy(CHARACTER_TABLE,new_id);
		free(new_id);
		return NULL;
	}

	/* Write position */
	if(!entry_write_string(CHARACTER_TABLE,new_id,map,CHARACTER_KEY_MAP,NULL)) {
		entry_destroy(CHARACTER_TABLE,new_id);
		free(new_id);
		return NULL;
	}

	if(!entry_write_int(CHARACTER_TABLE,new_id,x,CHARACTER_KEY_POS_X,NULL)) {
		entry_destroy(CHARACTER_TABLE,new_id);
		free(new_id);
		return NULL;
	}

	if(!entry_write_int(CHARACTER_TABLE,new_id,y,CHARACTER_KEY_POS_Y,NULL)) {
		entry_destroy(CHARACTER_TABLE,new_id);
		free(new_id);
		return NULL;
	}

	return new_id;
}

/***********************************************************************
***********************************************************************/
static void execute_aggro(context_t * agressor, context_t * target, char * script, int aggro_dist)
{
	int dist;
	char * param[] = { NULL,NULL,NULL };

	dist = context_distance(agressor,target);

	if(dist <= aggro_dist) {
		param[1] = "1";
	}
	else {
		param[1] = "0";
	}

	param[0] = target->id;
	param[2] = NULL;
	action_execute_script(agressor,script,param);

}
/***********************************************************************
Call aggro script for each context in every npc context aggro dist
***********************************************************************/
void character_update_aggro(context_t * agressor)
{
	context_t * target = NULL;
	context_t * npc = NULL;
	int aggro_dist;
	char * aggro_script;

	if( agressor == NULL ) {
		return;
	}

	if( agressor->map == NULL ) {
		return;
	}

	if( agressor->id == NULL ) {
		return;
	}

	if( agressor->luaVM == NULL ) {
		return;
	}

	/* If the current context is an NPC it might be an aggressor: compute its aggro */
	if( character_get_npc(agressor->id) ) {
		if(entry_read_int(CHARACTER_TABLE,agressor->id,&aggro_dist, CHARACTER_KEY_AGGRO_DIST,NULL)) {
			if(entry_read_string(CHARACTER_TABLE,agressor->id,&aggro_script, CHARACTER_KEY_AGGRO_SCRIPT,NULL)) {
				target = context_get_first();

				while( target != NULL ) {
					/* Skip current context */
					if( agressor == target) {
						target = target->next;
						continue;
					}
					if(target->id == NULL) {
						target = target->next;
						continue;
					}
					if(target->map == NULL) {
						target = target->next;
						continue;
					}
					/* Skip if not on the same map */
					if( strcmp(agressor->map,target->map) != 0 ) {
						target = target->next;
						continue;
					}
					execute_aggro(agressor,target,aggro_script,aggro_dist);
					target = target->next;
				}
				free(aggro_script);
			}
		}
	}

	/* Compute aggro of all other NPC to the current context */
	target = agressor;
	npc = context_get_first();

	while( npc != NULL ) {
		/* Skip current context */
		if( target == npc) {
			npc = npc->next;
			continue;
		}
		if(npc->id == NULL) {
			npc = npc->next;
			continue;
		}
		if(npc->map == NULL) {
			npc = npc->next;
			continue;
		}
		if( npc->luaVM == NULL ) {
			npc = npc->next;
			continue;
		}
		/* Skip if not on the same map */
		if( strcmp(npc->map,target->map) != 0 ) {
			npc = npc->next;
			continue;
		}
		if(!entry_read_int(CHARACTER_TABLE,npc->id,&aggro_dist, CHARACTER_KEY_AGGRO_DIST,NULL)) {
			npc = npc->next;
			continue;
		}
		if(!entry_read_string(CHARACTER_TABLE,npc->id,&aggro_script, CHARACTER_KEY_AGGRO_SCRIPT,NULL)) {
			npc = npc->next;
			continue;
		}

		execute_aggro(npc,target,aggro_script,aggro_dist);

		free(aggro_script);

		npc = npc->next;
	}
}
/*************************************************************
Move every context on the same coordinate as platform context
*************************************************************/
static void platform_move(context_t * platform,const char * map, int x, int y, int change_map)
{
	context_t * current = context_get_first();
	int is_platform;

	if(!entry_read_int(CHARACTER_TABLE,platform->id,&is_platform, CHARACTER_KEY_PLATFORM,NULL)) {
		return;
	}

	if(!is_platform) {
		return;
	}

	while(current) {
		if(current == platform) {
			current = current->next;
			continue;
		}
		if( platform->pos_x == current->pos_x && platform->pos_y == current->pos_y && !strcmp(platform->map, current->map) ) {
			context_set_map(current,map);
			context_set_pos_x(current,x);
			context_set_pos_y(current,y);

			entry_write_string(CHARACTER_TABLE,current->id,map,CHARACTER_KEY_MAP,NULL);
			entry_write_int(CHARACTER_TABLE,current->id,x,CHARACTER_KEY_POS_X,NULL);
			entry_write_int(CHARACTER_TABLE,current->id,y,CHARACTER_KEY_POS_Y,NULL);

			context_spread(current);
			if(change_map) {
				context_request_other_context(current);
			}
		}
		current = current->next;
	}
}

/******************************************************
return 0 if new position OK or if position has not changed.
return -1 if the position was not set (because tile not allowed or out of bound)
******************************************************/
int character_set_pos(context_t * ctx, const char * map, int x, int y)
{
	char ** event_id;
	char * script;
	char ** param = NULL;
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

		/* Th platform must be the last one to move */
		platform_move(ctx,map,x,y,change_map);

		context_set_map(ctx,map);
		context_set_pos_x(ctx,x);
		context_set_pos_y(ctx,y);

		entry_write_string(CHARACTER_TABLE,ctx->id,map,CHARACTER_KEY_MAP,NULL);
		entry_write_int(CHARACTER_TABLE,ctx->id,x,CHARACTER_KEY_POS_X,NULL);
		entry_write_int(CHARACTER_TABLE,ctx->id,y,CHARACTER_KEY_POS_Y,NULL);

		context_spread(ctx);
		if(change_map) {
			context_request_other_context(ctx);
		}

		event_id = map_get_event(map,x,y);

		if(event_id) {
			i = 0;
			while(event_id[i]) {
				if( !entry_read_string(MAP_TABLE,map,&script,MAP_ENTRY_EVENT_LIST,event_id[i],MAP_EVENT_SCRIPT,NULL) ) {
					i++;
					continue;
				}
				entry_read_list(MAP_TABLE,map,&param,MAP_ENTRY_EVENT_LIST,event_id[i],MAP_EVENT_PARAM,NULL);

				action_execute_script(ctx,script,param);

				free(script);
				deep_free(param);
				param=NULL;

				i++;
			}
			deep_free(event_id);
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
	if(!entry_write_int(CHARACTER_TABLE,id,npc,CHARACTER_KEY_NPC,NULL)) {
		return -1;
	}

	if(npc) {
		instantiate_npc(id);
	}

	return 0;
}

/*********************************************************
 Get NPC value.
 return 0 if not NPC
 return 1 if NPC
*********************************************************/
int character_get_npc(const char * id)
{
	int npc;

	if(!entry_read_int(CHARACTER_TABLE,id,&npc,CHARACTER_KEY_NPC,NULL)) {
		return 0;
	}

	return npc;
}

/*********************************************************
 Get "speak" parameter of character.
 It's a LUA script.
 returned value MUST BE FREED
*********************************************************/
char * character_get_speak(char * id)
{
	char * speak_script;
	
	if(!entry_read_string(CHARACTER_TABLE, id,&speak_script,CHARACTER_KEY_SPEAK,NULL)) {
		return NULL;
	}
	
	return speak_script;
}
