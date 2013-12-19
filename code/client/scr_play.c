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

void cb_select_map(void *arg)
{
	item_t * item = (item_t*)arg;
	context_t * ctx = context_get_list_first();

        ctx->selection.map_coord[0]= item->tile_x;
        ctx->selection.map_coord[1]= item->tile_y;
	network_send_context(ctx);
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
		item_set_tile(item,x,y);
		item_set_click_left(item,cb_select_map,item);
		x++;
		if(x>=ctx->map_x) {
			x=0;
			y++;
		}
		i++;
        }

        g_free(value);
}

void cb_select_sprite(void *arg)
{
	char * id = (char*)arg;

	context_t * ctx = context_get_list_first();
        ctx->selection.id= id;
	network_send_context(ctx);
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
		item_set_click_left(item,cb_select_sprite,ctx->id);

		ctx = ctx->next;
	}

	context_unlock_list();
}

void cb_action(void * arg)
{
	char * script = (char *)arg;

	network_send_action(context_get_list_first(),script,NULL);
}

/**********************************
Compose action icon
**********************************/
static void compose_action(context_t * ctx)
{
	char ** action_list = NULL;
	const char * text;
        const char * icon;
        const char * script;
	anim_t * anim;
	item_t * item;
	int sw = 0;
	int sh = 0;
	int x=0;

	SDL_GetRendererOutputSize(ctx->render,&sw,&sh);

	/* Read action list for current user */
        if(!read_list(CHARACTER_TABLE,ctx->id,&action_list,CHARACTER_KEY_ACTION,NULL)) {
                return;
        }

        while(*action_list != NULL ) {
                if(!read_string(ACTION_TABLE,*action_list,&text,ACTION_KEY_TEXT,NULL)) {
			action_list ++;
                        continue;
                }

                if(!read_string(ACTION_TABLE,*action_list,&icon,ACTION_KEY_ICON,NULL)) {
			action_list ++;
                        continue;
                }
		if(!read_string(ACTION_TABLE,*action_list,&script,ACTION_KEY_SCRIPT,NULL)) {
			action_list ++;
                        continue;
		}


                /* load image */
                anim = imageDB_get_anim(ctx, icon);
		if(anim == NULL) {
			action_list ++;
                        continue;
		}

		item = item_list_add(item_list);
		if(item_list == NULL) {
			item_list = item;
		}

		item_set_overlay(item,1);
		item_set_anim(item,x,sh-anim->h,anim);
		x += anim->w;
		item_set_click_left(item,cb_action,(void*)script);

		action_list ++;
        }

}

/**********************************
Compose select cursor
**********************************/
static void compose_select(context_t * ctx)
{
	item_t * item;
	anim_t * anim;
	int x;
	int y;

	anim = imageDB_get_anim(ctx,CURSOR_SPRITE_FILE);
	if(anim == NULL) {
		return;
	}

	x = ctx->selection.map_coord[0];
	y = ctx->selection.map_coord[1];

	if( x != -1 && y != -1) {
		item = item_list_add(item_list);
		if(item_list == NULL) {
			item_list = item;
		}

		/* get pixel coordiante from tile coordianate */
		x = x * ctx->tile_x;
		y = y * ctx->tile_y;

		/* Center on tile */
		x -= (anim->w-ctx->tile_x)/2;
                y -= (anim->h-ctx->tile_y)/2;

		item_set_anim(item,x,y,anim);
	}

	if( ctx->selection.id != NULL) {
		item = item_list_add(item_list);
		if(item_list == NULL) {
			item_list = item;
		}

                if(!read_int(CHARACTER_TABLE,ctx->selection.id,&x,CHARACTER_KEY_POS_X,NULL)) {
                        return;
                }
                if(!read_int(CHARACTER_TABLE,ctx->selection.id,&y,CHARACTER_KEY_POS_Y,NULL)) {
                        return;
                }

		/* Center on tile */
		x -= (anim->w-ctx->tile_x)/2;
                y -= (anim->h-ctx->tile_y)/2;

		item_set_anim(item,x,y,anim);
	}

}

/**********************************
Compose the character select screen
**********************************/
item_t * scr_play_compose(context_t * ctx)
{
	static int init = 1;

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
	compose_action(ctx);
	compose_select(ctx);

	sdl_set_virtual_x(ctx->cur_pos_x * ctx->tile_x + ctx->tile_x/2);
	sdl_set_virtual_y(ctx->cur_pos_y * ctx->tile_y + ctx->tile_y/2);

	sdl_free_keycb(NULL);
	sdl_add_keycb(SDL_SCANCODE_UP,key_up);
	sdl_add_keycb(SDL_SCANCODE_DOWN,key_down);
	sdl_add_keycb(SDL_SCANCODE_LEFT,key_left);
	sdl_add_keycb(SDL_SCANCODE_RIGHT,key_right);

	return item_list;
}
