/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013-2015 carabobz@gmail.com

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
char * character_create_from_template(context_t * ctx,const char * template,const char * map, int layer, int x, int y)
{
	char * new_id;
	char * templatename;
	char * fullname;

	new_id = file_new(CHARACTER_TABLE,NULL);

	templatename = strconcat(base_directory,"/",CHARACTER_TEMPLATE_TABLE,"/",template,NULL);
	fullname = strconcat(base_directory,"/",CHARACTER_TABLE,"/",new_id,NULL);
	file_copy(templatename,fullname);
	free(templatename);
	free(fullname);

	/* Check if new character is allowed to be created here */
	if(map_check_tile(ctx,new_id,map,layer,x,y) == 0) {
		entry_destroy(CHARACTER_TABLE,new_id);
		file_delete(CHARACTER_TABLE,new_id);
		free(new_id);
		return NULL;
	}

	/* Write position */
	if(!entry_write_string(CHARACTER_TABLE,new_id,map,CHARACTER_KEY_MAP,NULL)) {
		entry_destroy(CHARACTER_TABLE,new_id);
		file_delete(CHARACTER_TABLE,new_id);
		free(new_id);
		return NULL;
	}

	if(!entry_write_int(CHARACTER_TABLE,new_id,x,CHARACTER_KEY_POS_X,NULL)) {
		entry_destroy(CHARACTER_TABLE,new_id);
		file_delete(CHARACTER_TABLE,new_id);
		free(new_id);
		return NULL;
	}

	if(!entry_write_int(CHARACTER_TABLE,new_id,y,CHARACTER_KEY_POS_Y,NULL)) {
		entry_destroy(CHARACTER_TABLE,new_id);
		file_delete(CHARACTER_TABLE,new_id);
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
	} else {
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

	/* If the current context is an NPC it might be an aggressor: compute its aggro */
	if( character_get_npc(agressor->id) && agressor->luaVM != NULL) {
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
*************************************************************/
static void do_set_pos(context_t * ctx,const char * map, int x, int y, int change_map)
{
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
			do_set_pos(current,map,x,y,change_map);
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
	int width = x+1;
	int height = y+1;
	int warpx = FALSE;
	int warpy = FALSE;
	int ctx_layer = 0;
	char layer_name[SMALL_BUF];
	char buf[SMALL_BUF];
	char * coord[3];
	int ret_value;
	int layer;

	if(ctx == NULL) {
		return -1;
	}

	/* Do nothing if no move */
	if(!strcmp(ctx->map, map) && ctx->pos_x == x && ctx->pos_y == y) {
		return 0;
	}

	ctx_layer = 0;
	entry_read_int(CHARACTER_TABLE,ctx->id,&ctx_layer,CHARACTER_LAYER,NULL);
	sprintf(layer_name,"%s%d",MAP_KEY_LAYER,ctx_layer);

	entry_read_int(MAP_TABLE,map,&width,MAP_KEY_WIDTH,NULL);
	entry_read_int(MAP_TABLE,map,&height,MAP_KEY_HEIGHT,NULL);
	entry_read_int(MAP_TABLE,map,&warpx,MAP_KEY_WARP_X,NULL);
	entry_read_int(MAP_TABLE,map,&warpy,MAP_KEY_WARP_Y,NULL);

	/* Offscreen script */
	entry_read_string(MAP_TABLE,map,&script,MAP_OFFSCREEN,NULL);
	if(script != NULL &&
			( x < 0 || y < 0 || x >= width || y >= height ) ) {
		snprintf(buf,SMALL_BUF,"%d",x);
		coord[0] = strdup(buf);
		snprintf(buf,SMALL_BUF,"%d",y);
		coord[1] = strdup(buf);
		coord[2] = NULL;

		ret_value = action_execute_script(ctx,script,coord);

		free(coord[0]);
		free(coord[1]);
		free(script);

		return ret_value;
	}
	if( script ) {
		free(script);
	}

	/* Coordinates warping */
	if( x < 0 ) {
		if( warpy == FALSE) {
			return -1;
		}
		x = width-1;
	}
	if( y < 0 ) {
		if( warpy == FALSE) {
			return -1;
		}
		y = height-1;
	}
	if( x >= width ) {
		if( warpx == FALSE) {
			return -1;
		}
		x = 0;
	}
	if( y >= height ) {
		if( warpy == FALSE) {
			return -1;
		}
		y = 0;
	}

	/* Check if this character is allowed to go to the target tile */
	layer = ctx_layer;
	while ( layer >= 0 ) {
		ret_value = map_check_tile(ctx,ctx->id,map,layer,x,y);
		/* not allowed */
		if ( ret_value == 0 ) {
			return -1;
		}
		/* allowed */
		if ( ret_value == 1 ) {
			break;
		}
		layer--;
	}

	if( layer < 0 ) {
		return -1;
	}

	if( strcmp(ctx->map,map) ) {
		change_map = 1;
	}

	/* If this character is a platform, move all characters on it */
	platform_move(ctx,map,x,y,change_map);

	do_set_pos(ctx,map,x,y,change_map);

	event_id = map_get_event(map,ctx_layer,x,y);

	if(event_id) {
		i = 0;
		while(event_id[i]) {
			if( !entry_read_string(MAP_TABLE,map,&script,layer_name,MAP_ENTRY_EVENT_LIST,event_id[i],MAP_EVENT_SCRIPT,NULL) ) {
				i++;
				continue;
			}
			entry_read_list(MAP_TABLE,map,&param,layer_name,MAP_ENTRY_EVENT_LIST,event_id[i],MAP_EVENT_PARAM,NULL);

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
return -1 if error
*********************************************************/
int character_set_portrait(const char * id,const char * portrait)
{
	context_t * ctx;
	char * filename;
	int res;

	if(!entry_write_string(CHARACTER_TABLE,id,portrait,CHARACTER_KEY_PORTRAIT,NULL)) {
		return -1;
	}

	ctx = context_find(id);
	filename = strconcat(CHARACTER_TABLE,"/",id,NULL);
	res = network_send_file(ctx,filename);
	free(filename);

	return res;
}

/*********************************************************
 Get character portrait.
must be freed
*********************************************************/
char * character_get_portrait(const char * id)
{
	char * portrait;

	if(!entry_read_string(CHARACTER_TABLE,id,&portrait,CHARACTER_KEY_PORTRAIT,NULL)) {
		return NULL;
	}

	return portrait;
}

/*********************************************************
 Set AI script name
 return -1 on error
*********************************************************/
int character_set_ai_script(const char * id, const char * script_name)
{
	if(!entry_write_string(CHARACTER_TABLE,id,script_name,CHARACTER_KEY_AI,NULL)) {
		return -1;
	}

	return 0;
}

/*********************************************************
 Wake-up NPC. Execute it's AI script immediatly
 return -1 on error
*********************************************************/
int character_wake_up(const char * id)
{
	context_t * ctx;

	ctx = context_find(id);

	/* Wake up NPC */
	ctx->next_execution_time = 0;
	if( SDL_TryLockMutex (ctx->cond_mutex) == 0 ) {
		SDL_CondSignal (ctx->cond);
		SDL_UnlockMutex (ctx->cond_mutex);
	}

	return 0;
}
