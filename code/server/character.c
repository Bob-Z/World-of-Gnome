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

#include <glib.h>
#include <glib/gprintf.h>
#include <gio/gio.h>
#include "../common/common.h"
#include <dirent.h>
#include <string.h>
#include "npc.h"
#include "action.h"

extern context_t * context_list_start;

/*****************************/
void character_send_list(context_t * context)
{
	gchar ** character_list;
	gint i = 0;

	if(!read_list(USERS_TABLE, context->user_name,&character_list,USERS_CHARACTER_LIST,NULL)) {
		return;
	}

	while( character_list[i] != NULL ) {
		network_send_command(context, CMD_SEND_CHARACTER, g_utf8_strlen(character_list[i],-1)+1, character_list[i],FALSE);
		i++;
	}
	
	g_free(character_list);
}

/*****************************/
void character_user_send_list(context_t * context)
{
	gchar * data = NULL;
	guint data_size = 0;
	guint string_size = 0;
	gchar ** character_list;
	const gchar * type;
	const gchar * name;
	gint i;

	if(!read_list(USERS_TABLE, context->user_name,&character_list,USERS_CHARACTER_LIST,NULL)) {
		return;
	}

	i = 0;

	data = g_strdup("");
	while( character_list[i] != NULL ) {
		if(!read_string(CHARACTER_TABLE, character_list[i], &type, CHARACTER_KEY_TYPE,NULL)) {
			i++;
			continue;
		}

		if(!read_string(CHARACTER_TABLE, character_list[i], &name, CHARACTER_KEY_NAME,NULL)) {
			i++;
			continue;
		}

		/* add the name of the character to the network frame */
		string_size = g_utf8_strlen(character_list[i],-1)+1;
		data = g_realloc(data, data_size + string_size);
		g_memmove(data+data_size,character_list[i], string_size);
		data_size += string_size;

		/* add the type of the character to the network frame */
		string_size = g_utf8_strlen(type,-1)+1;
		data = g_realloc(data, data_size + string_size);
		g_memmove(data+data_size, type, string_size);
		data_size += string_size;

		/* add the type of the character to the network frame */
		string_size = g_utf8_strlen(name,-1)+1;
		data = g_realloc(data, data_size + string_size);
		g_memmove(data+data_size, name, string_size);
		data_size += string_size;

		i++;
	}

	g_free(character_list);

	/* Mark the end of the list */
	data = g_realloc(data, data_size + 1);
	data[data_size] = 0;
	data_size ++;

	network_send_command(context, CMD_SEND_USER_CHARACTER, data_size, data,FALSE);
	g_free(data);
}

/*****************************/
/* disconnect a character */
/* return -1 if fails */
gint character_disconnect( const gchar * id)
{
	context_t * ctx;

	ctx = context_find(id);
	/* For NPC */
	if( ctx->output_stream == NULL ) {
		context_set_connected(ctx,FALSE);
		/* Wake up NPC */
		if( g_mutex_trylock (ctx->cond_mutex) == TRUE ) {
			g_cond_signal (ctx->cond);
			g_mutex_unlock (ctx->cond_mutex);
		}
	}
	/* For player */
	/* TODO */
	return 0;
}

/******************************************************
 Create a new character based on the specified template
 return the id of the newly created character
 the returned string must be freed by caller
 return NULL if fails
*******************************************************/
gchar * character_create_from_template(const gchar * template)
{
        gchar * new_name;
        gchar * templatename;
        gchar * newfilename;
        GFile * templatefile;
        GFile * newfile;

        new_name = file_new(CHARACTER_TABLE);

        templatename = g_strconcat( g_getenv("HOME"),"/", base_directory, "/", CHARACTER_TEMPLATE_TABLE, "/", template,  NULL);
        templatefile = g_file_new_for_path(templatename);

        newfilename = g_strconcat( g_getenv("HOME"),"/", base_directory, "/", CHARACTER_TABLE, "/", new_name,  NULL);
        newfile = g_file_new_for_path(newfilename);

        if( g_file_copy(templatefile,newfile, G_FILE_COPY_OVERWRITE,NULL,NULL,NULL,NULL) == FALSE ) {
                g_free(new_name);
                return NULL;
        }

        return new_name;
}

/*****************************/
/* Call aggro script for each context in every npc context aggro dist */
void character_update_aggro(context_t * context)
{
        context_t * ctx = NULL;
        context_t * ctx2 = NULL;
        gint aggro_dist;
        gint dist;
        const gchar * aggro_script;
	gchar * param[] = { NULL,NULL };
	gint no_aggro = 1;

        ctx = context_list_start;

        if( ctx == NULL ) {
                return;
        }

	/*For each context: check dist of every other context on the same map*/
	/*For each context at aggro distance, execute the accro script */
	do {
		if(!ctx->id) {
			continue;
		}

		no_aggro = 1;
		if(!read_int(CHARACTER_TABLE,ctx->id,&aggro_dist, CHARACTER_KEY_AGGRO_DIST,NULL)) {
			continue;
		}

		if(!read_string(CHARACTER_TABLE,ctx->id,&aggro_script, CHARACTER_KEY_AGGRO_SCRIPT,NULL)) {
			continue;
		}

		ctx2 = context_list_start;

		do {
			/* Skip current context */
			if( ctx == ctx2) {
				continue;
			}

			if(!ctx2->id) {
				continue;
			}

			/* Skip if not on the same map */
			if( g_strcmp0(ctx->map,ctx2->map) != 0 ) {
				continue;
			}

			dist = context_distance(ctx,ctx2);

			if(dist <= aggro_dist) {
				param[0] = ctx2->id;
				action_execute_script(ctx,aggro_script,param);
				no_aggro = 0;
			}
		} while( (ctx2=ctx2->next)!= NULL );

		/* Notify if no aggro available */
		if( no_aggro ) {
			param[0] = "";
			action_execute_script(ctx,aggro_script,param);
		}
	} while( (ctx=ctx->next)!= NULL );

}

/******************************************************
return -1 if the postion was not set (because tile not allowed or out of bound)
******************************************************/
gint character_set_pos(context_t * ctx, gchar * map, gint x, gint y)
{
	const gchar ** event_id;
	const gchar * script;
	gchar ** param;
	int i;
	int change_map = 0;

	if(ctx == NULL) {
		return -1;
	}

	/* Check if this character is allowed to go to the target tile */
        if (map_check_tile(ctx->id,map,x,y) ) {

		if( g_strcmp0(ctx->map,map) ) {
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
				if( !read_string(MAP_TABLE,map,&script,MAP_ENTRY_EVENT_LIST,event_id[i],MAP_EVENT_SCRIPT,NULL) ) {
					i++;
					continue;
				}
				read_list(MAP_TABLE,map,&param,MAP_ENTRY_EVENT_LIST,event_id[i],MAP_EVENT_PARAM,NULL);

				action_execute_script(ctx,script,param);

				i++;
				g_free(param);
			}
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
gint character_set_npc(const gchar * id, gint npc)
{
	if(!write_int(CHARACTER_TABLE,id,npc,CHARACTER_KEY_NPC,NULL)){
		return -1;
	}

	if(npc) {
		instantiate_npc(id);
	}

	return 0;
}
