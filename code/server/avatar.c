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
void avatar_send_list(context_t * context)
{
	gchar ** avatar_list;
	gint i = 0;

	if(!read_list(USERS_TABLE, context->user_name,&avatar_list,USERS_CHARACTER_LIST,NULL)) {
		return;
	}

	while( avatar_list[i] != NULL ) {
		network_send_command(context, CMD_SEND_AVATAR, g_utf8_strlen(avatar_list[i],-1)+1, avatar_list[i],FALSE);
		i++;
	}
	
	g_free(avatar_list);
}

/*****************************/
void avatar_user_send_list(context_t * context)
{
	gchar * data = NULL;
	guint data_size = 0;
	guint string_size = 0;
	gchar ** avatar_list;
	const gchar * type;
	const gchar * name;
	gint i;

	if(!read_list(USERS_TABLE, context->user_name,&avatar_list,USERS_CHARACTER_LIST,NULL)) {
		return;
	}

	i = 0;

	data = g_strdup("");
	while( avatar_list[i] != NULL ) {
		if(!read_string(CHARACTER_TABLE, avatar_list[i], &type, CHARACTER_KEY_TYPE,NULL)) {
			i++;
			continue;
		}

		if(!read_string(CHARACTER_TABLE, avatar_list[i], &name, CHARACTER_KEY_NAME,NULL)) {
			i++;
			continue;
		}

		/* add the name of the avatar to the network frame */
		string_size = g_utf8_strlen(avatar_list[i],-1)+1;
		data = g_realloc(data, data_size + string_size);
		g_memmove(data+data_size,avatar_list[i], string_size);
		data_size += string_size;

		/* add the type of the avatar to the network frame */
		string_size = g_utf8_strlen(type,-1)+1;
		data = g_realloc(data, data_size + string_size);
		g_memmove(data+data_size, type, string_size);
		data_size += string_size;

		/* add the type of the avatar to the network frame */
		string_size = g_utf8_strlen(name,-1)+1;
		data = g_realloc(data, data_size + string_size);
		g_memmove(data+data_size, name, string_size);
		data_size += string_size;

		i++;
	}

	g_free(avatar_list);

	/* Mark the end of the list */
	data = g_realloc(data, data_size + 1);
	data[data_size] = 0;
	data_size ++;

	network_send_command(context, CMD_SEND_USER_AVATAR, data_size, data,FALSE);
	g_free(data);
}

/*****************************/
/* disconnect a character */
/*  if it's a npc, the NPC is deleted */
/* return -1 if fails */
gint avatar_disconnect( const gchar * id)
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

/*****************************/
/* Call aggro script for each context in every npc context aggro dist */
void avatar_update_aggro(context_t * context)
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
gint avatar_set_pos(context_t * ctx, gchar * map, gint x, gint y)
{
	const gchar ** script;
	int i;
	int change_map = 0;

	/* Check if this avatar is allowed to go to the target tile */
        if (map_check_tile(ctx,map,x,y) ) {

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

		script = map_get_event(map,x,y);
		
		if(script) {
			i = 0;
			while(script[i]) {
				action_execute_script(ctx,script[i],NULL);
				i++;
			}

			g_free(script);
		}

		avatar_update_aggro(ctx);
		return 0;
        }
	return -1;
}


