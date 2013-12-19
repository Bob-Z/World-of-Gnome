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

#include <gtk/gtk.h>
#include "config.h"
#include "../common/common.h"
#include "win_game.h"
#include <glib.h>
#include <glib/gstdio.h>
#include "imageDB.h"
#include "file.h"
#include "anim.h"
#include "item.h"
#include "sdl.h"

//static pthread_mutex_t character_mutex = PTHREAD_MUTEX_INITIALIZER;
static item_t * item_list = NULL;

//Keynoard callback

static void key_up(void * arg)
{
	context_t * ctx = context_get_list_first();

	network_send_action(ctx,"move_up.lua",NULL);
}
static void key_down(void * arg)
{
	context_t * ctx = context_get_list_first();

	network_send_action(ctx,"move_down.lua",NULL);
}
static void key_left(void * arg)
{
	context_t * ctx = context_get_list_first();

	network_send_action(ctx,"move_left.lua",NULL);
}
static void key_right(void * arg)
{
	context_t * ctx = context_get_list_first();

	network_send_action(ctx,"move_right.lua",NULL);
}

/**********************************
Compose the characters map
**********************************/
static void compose_map(context_t * ctx)
{
	int i;
	char ** value = NULL;
	int x = 0;
        int y = 0;
	const char * tile_image;
	anim_t * anim;
	item_t * item;

	/* description of the map */
        if( ctx->map_x == -1 ) {
                if(!read_int(MAP_TABLE, ctx->map, &i,MAP_KEY_SIZE_X,NULL)) {
                        return;
                }
                context_set_map_x(ctx, i);
        }
        if( ctx->map_y == -1 ) {
                if(!read_int(MAP_TABLE, ctx->map, &i,MAP_KEY_SIZE_Y,NULL)) {
                        return;
                }
                context_set_map_y( ctx,i);
        }
        if( ctx->tile_x == -1 ) {
                if(!read_int(MAP_TABLE, ctx->map, &i,MAP_KEY_TILE_SIZE_X,NULL)){
                        return;
                }
                context_set_tile_x( ctx, i);
        }
        if( ctx->tile_y == -1 ) {
                if(!read_int(MAP_TABLE, ctx->map,&i,MAP_KEY_TILE_SIZE_Y,NULL)) {
                        return;
                }
                context_set_tile_y( ctx, i);
        }
        if(!read_list(MAP_TABLE, ctx->map, &value,MAP_KEY_SET,NULL)) {
                return;
        }


	/* Parse map string */
	i=0;
        while(value[i] != NULL ) {
		if(!read_string(TILE_TABLE,value[i],&tile_image,TILE_KEY_IMAGE,NULL)) {
                break;
		}
#if 0
		/* Save description for caller */
		read_string(TILE_TABLE,tile_name,description,TILE_KEY_TEXT,NULL);
#endif
		item = item_list_add(item_list);
		if(item_list == NULL) {
			item_list = item;
		}

		anim = imageDB_get_anim(ctx,tile_image);
		item_set_anim(item,x*ctx->tile_x,y*ctx->tile_y,anim);
		x++;
		if(x>=ctx->map_x) {
			x=0;
			y++;
		}
		i++;
        }

        g_free(value);
}

/**********************************
Compose sprites
**********************************/
static void compose_sprite(context_t * ctx)
{
	const char * sprite_name = NULL;
	anim_t * anim;
	item_t * item;
	int x;
	int y;
	int ox;
	int oy;
	Uint32 timer;

	context_lock_list();

        while(ctx != NULL ) {
		/* compute the sprite file name */
		if(!read_string(CHARACTER_TABLE,ctx->id,&sprite_name,CHARACTER_KEY_SPRITE,NULL)) {
			werr(LOGDEV,"ID=%s. Can't read sprite name for \"%s\" type",ctx->id,ctx->type);
			break;;
		}

		anim = imageDB_get_anim(ctx,sprite_name);

		item = item_list_add(item_list);
		if(item_list == NULL) {
			item_list = item;
		}

		timer = SDL_GetTicks();

		if( ctx->pos_tick == 0 ) {
			ctx->cur_pos_x = ctx->pos_x;
			ctx->cur_pos_y = ctx->pos_y;
			ctx->pos_tick = 1;
		}

		/* If previous animation has ended */
		if( ctx->pos_tick + VIRTUAL_ANIM_DURATION < timer ) {
			ctx->old_pos_x = ctx->cur_pos_x;
			ctx->old_pos_y = ctx->cur_pos_y;
		/* Detect sprite movement, initiate animation */
			if(ctx->pos_x != ctx->old_pos_x||ctx->pos_y != ctx->old_pos_y){
				ctx->pos_tick = timer;
				ctx->cur_pos_x = ctx->pos_x;
				ctx->cur_pos_y = ctx->pos_y;
			}
		}

		/* Get position in pixel */
		x = ctx->cur_pos_x * ctx->tile_x;
		y = ctx->cur_pos_y * ctx->tile_y;
		ox = ctx->old_pos_x * ctx->tile_x;
		oy = ctx->old_pos_y * ctx->tile_y;

		/* Center sprite on tile */
		x -= (anim->w-ctx->tile_x)/2;
		y -= (anim->h-ctx->tile_y)/2;
		ox -= (anim->w-ctx->tile_x)/2;
		oy -= (anim->h-ctx->tile_y)/2;

		item_set_smooth_anim(item,x,y,ox,oy,ctx->pos_tick,anim);

		ctx = ctx->next;
	}

	context_unlock_list();
}

/**********************************
Compose the character select screen
**********************************/
item_t * scr_play_compose(context_t * ctx)
{
	static int init = 1;

	wlog(LOGDEBUG,"Composing play screen\n");

	if(item_list) {
		item_list_free(item_list);
		item_list = NULL;
	}

	if(ctx->map == NULL ) {
		if(!context_update_from_file(ctx)) {
			return NULL;
		}
	}

	if(init) {
		/* Register this character to receive server notifications */
		network_send_context(ctx);
		init = 0;
	}

	compose_map(ctx);

	compose_sprite(ctx);

	sdl_set_virtual_x(ctx->pos_x * ctx->tile_x + ctx->tile_x/2);
	sdl_set_virtual_y(ctx->pos_y * ctx->tile_y + ctx->tile_y/2);

	sdl_free_keycb(NULL);
	sdl_add_keycb(SDL_SCANCODE_UP,key_up);
	sdl_add_keycb(SDL_SCANCODE_DOWN,key_down);
	sdl_add_keycb(SDL_SCANCODE_LEFT,key_left);
	sdl_add_keycb(SDL_SCANCODE_RIGHT,key_right);

	return item_list;
}
