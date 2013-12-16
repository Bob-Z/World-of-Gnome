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
static int num_item = 0;

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
		anim = imageDB_get_anim(ctx,tile_image);
//TODO Free anim when freeing the item list 

		num_item++;
		item_list = realloc(item_list,sizeof(item_t)*num_item);
		item_init(&item_list[num_item-1]);
		item_set_anim(&item_list[num_item-1],x*ctx->tile_x,y*ctx->tile_y,anim);
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
static void compose_sprite(context_t * context)
{
	const char * sprite_name = NULL;
	anim_t * anim;
	context_t * ctx;

	context_lock_list();

	ctx = context;
        while(ctx != NULL ) {
		/* compute the sprite file name */
		if(!read_string(CHARACTER_TABLE,ctx->id,&sprite_name,CHARACTER_KEY_SPRITE,NULL)) {
			werr(LOGUSER,"Can't read sprite name for \"%s\" type",context->type);
			break;;
		}

		anim = imageDB_get_anim(ctx,sprite_name);

		num_item++;
		item_list = realloc(item_list,sizeof(item_t)*num_item);
		item_init(&item_list[num_item-1]);
		item_set_anim(&item_list[num_item-1],ctx->pos_x*ctx->tile_x,ctx->pos_y*ctx->tile_y,anim);

		ctx = ctx->next;
	}

	context_unlock_list();
}

/**********************************
Compose the character select screen
**********************************/
item_t * scr_play_compose(context_t * ctx)
{
	wlog(LOGDEBUG,"Composing play screen\n");

	if(item_list) {
		free(item_list);
		item_list = NULL;
		num_item = 0;
	}

	if(ctx->map == NULL ) {
		if(!context_update_from_file(ctx)) {
			return NULL;
		}
	}

	compose_map(ctx);

	compose_sprite(ctx);

	item_set_last(&item_list[num_item-1],1);

	sdl_set_virtual_x(ctx->pos_x * ctx->tile_x + ctx->tile_x/2);
	sdl_set_virtual_y(ctx->pos_y * ctx->tile_y + ctx->tile_y/2);
#if 0
	pthread_mutex_lock(&character_mutex);

	item_list = malloc(character_num*sizeof(item_t)*3);

	/* Create item list */
	for(i=0;i<character_num;i++) {
		/* Character picture */
		item_init(&item_list[i*3]);
		item_set_anim(&item_list[i*3],x,max_h/2-character_list[i].anim->h/2,character_list[i].anim);
		item_set_click_left(&item_list[i*3],cb_left_click,(void *)&item_list[i*3]);

		x += character_list[i].anim->w + BORDER;
		/* character name */
		item_init(&item_list[i*3+1]);
		item_set_string(&item_list[i*3+1],character_list[i].name);
		item_set_font(&item_list[i*3+1],TTF_OpenFont(FONT, FONT_SIZE));
		/* display string just above the picture */
		item_set_frame(&item_list[i*3+1],item_list[i*3].rect.x + item_list[i*3].rect.w/2, item_list[i*3].rect.y-FONT_SIZE/2,NULL);

		/* character type */
		item_init(&item_list[i*3+2]);
		item_set_string(&item_list[i*3+2],character_list[i].type);
		item_set_font(&item_list[i*3+2],TTF_OpenFont(FONT, FONT_SIZE));
		/* display string just below the picture */
		item_set_frame(&item_list[i*3+2],item_list[i*3].rect.x + item_list[i*3].rect.w/2, item_list[i*3].rect.y+item_list[i*3].rect.h+FONT_SIZE/2,NULL);
	}
	item_set_last(&item_list[(i*3)-1],1);

	if(init) {
		cb_left_click(&item_list[0]);
		init = 0;
	}

	pthread_mutex_unlock(&character_mutex);
#endif

	return item_list;
}

/*************************
 add a character to the list
the data is a list a 3 strings, the first string is the id of the character (its file name) the second one is the type of the character, the third is the name of the character.
the list ends with an empty string
*************************/
#if 0
void scr_select_add_user_character(context_t * context, char * data)
{
	char * current_string = data;

	pthread_mutex_lock(&character_mutex);

	while(current_string[0] != 0) {

		character_num++;

		character_list = realloc(character_list,sizeof(character_t)*character_num);

		character_list[character_num-1].id = strdup(current_string);
		current_string += strlen(current_string)+1;
		character_list[character_num-1].type = strdup(current_string);
		current_string += strlen(current_string)+1;
		character_list[character_num-1].name = strdup(current_string);
		current_string += strlen(current_string)+1;
		character_list[character_num-1].anim = NULL;
	}

	pthread_mutex_unlock(&character_mutex);

	wlog(LOGDEV,"Received character %s of type %s\n",character_list[character_num-1].name,character_list[character_num-1].type);
}
#endif
