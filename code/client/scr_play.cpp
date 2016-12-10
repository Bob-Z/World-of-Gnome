/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013-2016 carabobz@gmail.com

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

#include "config.h"
#include "../common/common.h"
#include "imageDB.h"
#include "file.h"
#include "../sdl_item/anim.h"
#include "../sdl_item/item.h"
#include "../sdl_item/sdl.h"
#include "screen.h"
#include "textview.h"
#include "network_client.h"
#include "ui_play.h"
#include "option_client.h"
#include <limits.h>

#define ITEM_FONT "Ubuntu-C.ttf"
#define ITEM_FONT_SIZE 15

#define NORTH (1<<0)
#define SOUTH (1<<1)
#define EAST (1<<2)
#define WEST (1<<3)

#define ALIGN_CENTER	0
#define ALIGN_LOWER	1

#define MAX_LAYER	100

static item_t * item_list = NULL;
static bool change_map = false;
static int init = true;
static int current_map_x = -1;
static int current_map_y = -1;
static option_t * option;
static layer_t * default_layer = NULL;
static char * sfx = NULL;

/**********************************
**********************************/
void scr_play_init(int init_value)
{
	context_t * ctx;

	init = init_value;

	if(init_value == true) {
		ctx = context_get_player();
		sfx_stop(ctx,sfx);
	}
}


/**********************************
**********************************/
int scr_play_get_current_x()
{
	return current_map_x;
}

/**********************************
**********************************/
int scr_play_get_current_y()
{
	return current_map_y;
}

/**********************************
**********************************/
static void cb_select_sprite(void *arg)
{
	char * id = (char*)arg;
	context_t * ctx = context_get_player();

	network_send_action(ctx,option->action_select_character,id,NULL);
}

/**********************************
**********************************/
static void cb_redo_sprite(void *arg)
{
	char * script = NULL;
	char * last_action = NULL;

	cb_select_sprite(arg);

	last_action = ui_play_get_last_action();
	if(last_action) {
		script = strdup(last_action);
		ui_play_cb_action(script);
		free(script);
	}
}

/**********************************
**********************************/
static void cb_zoom(Uint32 y,Uint32 unused)
{
	double zoom;

	zoom = sdl_get_virtual_z();

	sdl_set_virtual_z(zoom*1.1);
}

/**********************************
**********************************/
static void cb_unzoom(Uint32 y,Uint32 unused)
{
	double zoom;

	zoom = sdl_get_virtual_z();

	sdl_set_virtual_z(zoom/1.1);
}

/**********************************
Select sprite image to display
Return NULL if no sprite can be found
Returned anim_t ** must be FREED
**********************************/
static anim_t ** select_sprite(context_t * ctx, const char * image_file_name)
{
	anim_t ** sprite;
	char * sprite_name = NULL;
	char ** sprite_list = NULL;
	const char * sprite_name_array[2] = { NULL, NULL };
	context_t * player_context = context_get_player();

	if( image_file_name ) {
		sprite_name_array[0] = image_file_name;
		sprite = imageDB_get_anim_array(player_context,sprite_name_array);
		return sprite;
	}

	/* Try to find a sprite depending on the orientation */
	if( ctx->orientation & NORTH ) {
		entry_read_string(CHARACTER_TABLE,ctx->id,&sprite_name,CHARACTER_KEY_DIR_N_SPRITE,NULL);
	}
	if( (ctx->orientation & SOUTH) && sprite_name == NULL) {
		entry_read_string(CHARACTER_TABLE,ctx->id,&sprite_name,CHARACTER_KEY_DIR_S_SPRITE,NULL);
	}
	if( (ctx->orientation & EAST) && sprite_name == NULL) {
		entry_read_string(CHARACTER_TABLE,ctx->id,&sprite_name,CHARACTER_KEY_DIR_E_SPRITE,NULL);
	}
	if( (ctx->orientation & WEST) && sprite_name == NULL) {
		entry_read_string(CHARACTER_TABLE,ctx->id,&sprite_name,CHARACTER_KEY_DIR_W_SPRITE,NULL);
	}

	if( sprite_name ) {
		if( sprite_name[0] != 0 ) {
			sprite_name_array[0] = sprite_name;
			sprite = imageDB_get_anim_array(player_context,sprite_name_array);
			free(sprite_name);
			return sprite;
		}
		free(sprite_name);
	}

	/* Try sprite lists */
	if( ctx->orientation & NORTH ) {
		entry_read_list(CHARACTER_TABLE,ctx->id,&sprite_list,CHARACTER_KEY_DIR_N_SPRITE,NULL);
	}
	if( (ctx->orientation & SOUTH) && sprite_list == NULL) {
		entry_read_list(CHARACTER_TABLE,ctx->id,&sprite_list,CHARACTER_KEY_DIR_S_SPRITE,NULL);
	}
	if( (ctx->orientation & EAST) && sprite_list == NULL) {
		entry_read_list(CHARACTER_TABLE,ctx->id,&sprite_list,CHARACTER_KEY_DIR_E_SPRITE,NULL);
	}
	if( (ctx->orientation & WEST) && sprite_list == NULL) {
		entry_read_list(CHARACTER_TABLE,ctx->id,&sprite_list,CHARACTER_KEY_DIR_W_SPRITE,NULL);
	}

	if( sprite_list ) {
		sprite = imageDB_get_anim_array(player_context,(const char **)sprite_list);
		deep_free(sprite_list);
		return sprite;
	}

	/* try default sprite file */
	if(entry_read_string(CHARACTER_TABLE,ctx->id,&sprite_name,CHARACTER_KEY_SPRITE,NULL) == RET_OK ) {
		if( sprite_name[0] != 0 ) {
			sprite_name_array[0] = sprite_name;
			sprite = imageDB_get_anim_array(player_context,sprite_name_array);
			free(sprite_name);
			return sprite;
		}
		free(sprite_name);
	}
	/* try default sprite list */
	if(entry_read_list(CHARACTER_TABLE,ctx->id,&sprite_list,CHARACTER_KEY_SPRITE,NULL) == RET_OK ) {
		sprite = imageDB_get_anim_array(player_context,(const char **)sprite_list);
		deep_free(sprite_list);
		return sprite;
	}

	werr(LOGDEV,"Can't read sprite name for \"%s\"",ctx->id);
	return NULL;
}

/**********************************
Select sprite image to display when sprite is moving
Return NULL if no sprite can be found
**********************************/
static anim_t ** select_sprite_move(context_t * ctx, const char * image_file_name)
{
	anim_t ** sprite;
	char * sprite_name = NULL;
	char ** sprite_list = NULL;
	const char * sprite_name_array[2] = { NULL, NULL };
	context_t * player_context = context_get_player();

	if( image_file_name ) {
		sprite_name_array[0] = image_file_name;
		sprite = imageDB_get_anim_array(player_context,sprite_name_array);
		return sprite;
	}

	/* Try to find a sprite depending on the orientation */
	if( ctx->orientation & NORTH ) {
		entry_read_string(CHARACTER_TABLE,ctx->id,&sprite_name,CHARACTER_KEY_MOV_N_SPRITE,NULL);
	}
	if( (ctx->orientation & SOUTH) && sprite_name == NULL) {
		entry_read_string(CHARACTER_TABLE,ctx->id,&sprite_name,CHARACTER_KEY_MOV_S_SPRITE,NULL);
	}
	if( (ctx->orientation & EAST) && sprite_name == NULL) {
		entry_read_string(CHARACTER_TABLE,ctx->id,&sprite_name,CHARACTER_KEY_MOV_E_SPRITE,NULL);
	}
	if( (ctx->orientation & WEST) && sprite_name == NULL) {
		entry_read_string(CHARACTER_TABLE,ctx->id,&sprite_name,CHARACTER_KEY_MOV_W_SPRITE,NULL);
	}

	if( sprite_name ) {
		sprite_name_array[0] = sprite_name;
		sprite = imageDB_get_anim_array(player_context,sprite_name_array);
		free(sprite_name);
		return sprite;
	}

	/* Try sprite lists */
	if( ctx->orientation & NORTH ) {
		entry_read_list(CHARACTER_TABLE,ctx->id,&sprite_list,CHARACTER_KEY_MOV_N_SPRITE,NULL);
	}
	if( (ctx->orientation & SOUTH) && sprite_list == NULL) {
		entry_read_list(CHARACTER_TABLE,ctx->id,&sprite_list,CHARACTER_KEY_MOV_S_SPRITE,NULL);
	}
	if( (ctx->orientation & EAST) && sprite_list == NULL) {
		entry_read_list(CHARACTER_TABLE,ctx->id,&sprite_list,CHARACTER_KEY_MOV_E_SPRITE,NULL);
	}
	if( (ctx->orientation & WEST) && sprite_list == NULL) {
		entry_read_list(CHARACTER_TABLE,ctx->id,&sprite_list,CHARACTER_KEY_MOV_W_SPRITE,NULL);
	}

	if( sprite_list ) {
		sprite = imageDB_get_anim_array(player_context,(const char **)sprite_list);
		deep_free(sprite_list);
		return sprite;
	}

	return NULL;
}
/**********************************
Draw a single sprite
if image_file_name is not NULL, this file is used as an image rather than the normal sprite image
**********************************/
static void set_up_sprite(context_t * ctx, const char * image_file_name)
{
	anim_t ** sprite_list;
	anim_t ** sprite_move_list;
	item_t * item;
	int px;
	int py;
	int opx;
	int opy;
	Uint32 current_time;
	int angle;
	int flip;
	int force_flip;
	int move_status;
	char * zoom_str = NULL;
	double zoom = 1.0;
	int sprite_align = ALIGN_CENTER;
	int sprite_offset_y = 0;
	bool force_position = false;

	context_t * player_context = context_get_player();

	if( ctx->map == NULL ) {
		return;
	}
	if( ctx->in_game == false ) {
		return;
	}
	if( strcmp(ctx->map,player_context->map)) {
		return;
	}

	item = item_list_add(&item_list);

	current_time = sdl_get_global_time();

	// Force position when the player has changed map
	if( change_map == true ) {
		ctx->move_start_tick = current_time;
		ctx->animation_tick = current_time;
		force_position = true;
	}
	// Force position when this context has changed map
	if( ctx->change_map == true ) {
		ctx->move_start_tick = current_time;
		ctx->animation_tick = current_time;
		ctx->change_map = false;
		force_position = true;
	}

	if( ctx->animation_tick == 0 ) {
		ctx->animation_tick = current_time;
	}

	if( ctx->cur_pos_px == INT_MAX || ctx->cur_pos_py == INT_MAX ) {
		force_position = true;
	}

	// Detect sprite movement, initiate animation
	if( ctx->pos_changed && force_position == false ) {
		ctx->pos_changed = false;
		ctx->move_start_tick = current_time;
		ctx->start_pos_px = ctx->cur_pos_px;
		ctx->start_pos_py = ctx->cur_pos_py;

		/* flip need to remember previous direction to avoid resetting a
		   east -> west flip when a sprite goes to north for instance.
		   On the contrary rotation must not remember previous state, or
		   the rotation will be wrong.
		   Hence the distinction between orientation (no memory) and
		   direction (memory). */
		ctx->orientation = 0;
		// Compute direction
		if( ctx->pos_tx > ctx->prev_pos_tx ) {
			ctx->direction &= ~WEST;
			ctx->direction |= EAST;
			ctx->orientation |= EAST;
		}
		if( ctx->pos_tx < ctx->prev_pos_tx ) {
			ctx->direction &= ~EAST;
			ctx->direction |= WEST;
			ctx->orientation |= WEST;
		}
		if( ctx->pos_ty > ctx->prev_pos_ty ) {
			ctx->direction &= ~NORTH;
			ctx->direction |= SOUTH;
			ctx->orientation |= SOUTH;
		}
		if( ctx->pos_ty < ctx->prev_pos_ty ) {
			ctx->direction &= ~SOUTH;
			ctx->direction |= NORTH;
			ctx->orientation |= NORTH;
		}
	}

	// Select sprite to display
	sprite_list = select_sprite(ctx,image_file_name);
	if( sprite_list == NULL ) {
		return;
	}
	if( sprite_list[0] == NULL ) {
		free(sprite_list);
		return;
	}
	sprite_move_list = select_sprite_move(ctx,image_file_name);

	// Get position in pixel
	px = map_t2p_x(ctx->pos_tx,ctx->pos_ty,default_layer);
	py = map_t2p_y(ctx->pos_tx,ctx->pos_ty,default_layer);

	// Get per sprite zoom
	if(entry_read_string(CHARACTER_TABLE,ctx->id,&zoom_str,CHARACTER_KEY_ZOOM,NULL) == RET_OK ) {
		zoom = atof(zoom_str);
		free(zoom_str);
	}

	// Align sprite on tile
	entry_read_int(CHARACTER_TABLE,ctx->id,&sprite_align,CHARACTER_KEY_ALIGN,NULL);
	if( sprite_align == ALIGN_CENTER ) {
		px -= ((sprite_list[0]->w*default_layer->map_zoom*zoom)-default_layer->tile_width)/2;
		py -= ((sprite_list[0]->h*default_layer->map_zoom*zoom)-default_layer->tile_height)/2;
	}
	if( sprite_align == ALIGN_LOWER ) {
		px -= ((sprite_list[0]->w*default_layer->map_zoom*zoom)-default_layer->tile_width)/2;
		py -= (sprite_list[0]->h*default_layer->map_zoom*zoom)-default_layer->tile_height ;
	}

	// Add Y offset
	entry_read_int(CHARACTER_TABLE,ctx->id,&sprite_offset_y,CHARACTER_KEY_OFFSET_Y,NULL);
	py += sprite_offset_y;

	// Set sprite to item
	item_set_anim_start_tick(item,ctx->animation_tick);

	if( force_position == true ) {
		ctx->start_pos_px = px;
		ctx->cur_pos_px = px;
		ctx->start_pos_py = py;
		ctx->cur_pos_py = py;
	}

	opx = ctx->start_pos_px;
	opy = ctx->start_pos_py;

	item_set_move(item,opx,opy,px,py,ctx->move_start_tick,VIRTUAL_ANIM_DURATION);
	item_set_save_coordinate(item,&ctx->cur_pos_px,&ctx->cur_pos_py);
	item_set_anim_array(item,sprite_list);
	free(sprite_list);
	item_set_anim_move_array(item,sprite_move_list);
	free(sprite_move_list);

	// Get rotation configuration
	angle = 0;
	if( ctx->orientation & NORTH && ctx->orientation & EAST ) {
		entry_read_int(CHARACTER_TABLE,ctx->id,&angle,CHARACTER_KEY_DIR_NE_ROT,NULL);
		item_set_angle(item,(double)angle);
	} else if ( ctx->orientation & SOUTH && ctx->orientation & EAST ) {
		entry_read_int(CHARACTER_TABLE,ctx->id,&angle,CHARACTER_KEY_DIR_SE_ROT,NULL);
		item_set_angle(item,(double)angle);
	} else if ( ctx->orientation & SOUTH && ctx->orientation & WEST ) {
		entry_read_int(CHARACTER_TABLE,ctx->id,&angle,CHARACTER_KEY_DIR_SW_ROT,NULL);
		item_set_angle(item,(double)angle);
	} else if ( ctx->orientation & NORTH && ctx->orientation & WEST ) {
		entry_read_int(CHARACTER_TABLE,ctx->id,&angle,CHARACTER_KEY_DIR_NW_ROT,NULL);
		item_set_angle(item,(double)angle);
	} else if ( ctx->orientation & NORTH ) {
		entry_read_int(CHARACTER_TABLE,ctx->id,&angle,CHARACTER_KEY_DIR_N_ROT,NULL);
		item_set_angle(item,(double)angle);
	} else if ( ctx->orientation & SOUTH ) {
		entry_read_int(CHARACTER_TABLE,ctx->id,&angle,CHARACTER_KEY_DIR_S_ROT,NULL);
		item_set_angle(item,(double)angle);
	} else if ( ctx->orientation & WEST ) {
		entry_read_int(CHARACTER_TABLE,ctx->id,&angle,CHARACTER_KEY_DIR_W_ROT,NULL);
		item_set_angle(item,(double)angle);
	} else if ( ctx->orientation & EAST ) {
		entry_read_int(CHARACTER_TABLE,ctx->id,&angle,CHARACTER_KEY_DIR_E_ROT,NULL);
		item_set_angle(item,(double)angle);
	}

	// Get flip configuration
	force_flip = 0;
	entry_read_int(CHARACTER_TABLE,ctx->id,&force_flip,CHARACTER_KEY_FORCE_FLIP,NULL);
	move_status = ctx->direction;
	if( force_flip == true ) {
		move_status = ctx->orientation;
	}

	flip = 0;
	if( angle == 0 ) {
		if( move_status & NORTH ) {
			entry_read_int(CHARACTER_TABLE,ctx->id,&flip,CHARACTER_KEY_DIR_N_FLIP,NULL);
		}
		if( move_status & SOUTH ) {
			entry_read_int(CHARACTER_TABLE,ctx->id,&flip,CHARACTER_KEY_DIR_S_FLIP,NULL);
		}
		if( move_status & WEST ) {
			entry_read_int(CHARACTER_TABLE,ctx->id,&flip,CHARACTER_KEY_DIR_W_FLIP,NULL);
		}
		if( move_status & EAST ) {
			entry_read_int(CHARACTER_TABLE,ctx->id,&flip,CHARACTER_KEY_DIR_E_FLIP,NULL);
		}

		switch(flip) {
		case 1:
			item_set_flip(item,SDL_FLIP_HORIZONTAL);
			break;
		case 2:
			item_set_flip(item,SDL_FLIP_VERTICAL);
			break;
		case 3:
			item_set_flip(item,SDL_FLIP_HORIZONTAL|SDL_FLIP_VERTICAL);
			break;
		default:
			item_set_flip(item,SDL_FLIP_NONE);
		}
	}

	item_set_click_left(item,cb_select_sprite,ctx->id,NULL);
	item_set_click_right(item,cb_redo_sprite,item,NULL);

	item_set_zoom_x(item,zoom * default_layer->map_zoom );
	item_set_zoom_y(item,zoom * default_layer->map_zoom );
}

/**********************************
Compose sprites
**********************************/
static void compose_sprite(context_t * ctx,int layer_index)
{
	int layer;

	context_lock_list();

	while(ctx != NULL ) {
		layer = 0;
		entry_read_int(CHARACTER_TABLE,ctx->id,&layer,CHARACTER_LAYER,NULL);

		if( layer == layer_index ) {
			set_up_sprite(ctx,NULL);
		}
		ctx = ctx->next;
	}

	context_unlock_list();
}

/**********************************
Compose item on map
**********************************/
static void compose_item(context_t * ctx,int layer_index)
{
	char * sprite_name = NULL;
	int sprite_align = ALIGN_CENTER;
	int sprite_offset_y = 0;
	anim_t * anim;
	item_t * item;
	int x;
	int y;
	int temp_x;
	int temp_y;
	char ** item_id;
	int i;
	static TTF_Font * font = NULL;
	char * mytemplate;
	int quantity;
	char buf[SMALL_BUF];
	char layer_name[SMALL_BUF];

	sprintf(layer_name,"%s%d",MAP_KEY_LAYER,layer_index);

	if(entry_get_group_list(MAP_TABLE,ctx->map,&item_id,layer_name,MAP_ENTRY_ITEM_LIST,NULL) == RET_NOK ) {
		return;
	}

	font = font_get(ctx,ITEM_FONT, ITEM_FONT_SIZE);

	i=0;
	while( item_id[i] != NULL ) {
		sprite_align = ALIGN_CENTER;

		if(entry_read_int(MAP_TABLE,ctx->map,&x,layer_name,MAP_ENTRY_ITEM_LIST,item_id[i],MAP_ITEM_POS_X,NULL) == RET_NOK ) {
			i++;
			continue;
		}

		if(entry_read_int(MAP_TABLE,ctx->map,&y,layer_name,MAP_ENTRY_ITEM_LIST,item_id[i],MAP_ITEM_POS_Y,NULL) == RET_NOK ) {
			i++;
			continue;
		}

		mytemplate = item_is_resource(item_id[i]);

		if ( mytemplate == NULL ) {
			if(entry_read_string(ITEM_TABLE,item_id[i],&sprite_name,ITEM_SPRITE,NULL) == RET_NOK ) {
				i++;
				continue;
			}
			entry_read_int(ITEM_TABLE,item_id[i],&sprite_align,ITEM_ALIGN,NULL);
			entry_read_int(ITEM_TABLE,item_id[i],&sprite_offset_y,ITEM_OFFSET_Y,NULL);
		} else {
			if(entry_read_string(ITEM_TEMPLATE_TABLE,mytemplate,&sprite_name,ITEM_SPRITE,NULL) == RET_NOK ) {
				free(mytemplate);
				i++;
				continue;
			}
			entry_read_int(ITEM_TEMPLATE_TABLE,mytemplate,&sprite_align,ITEM_ALIGN,NULL);
			entry_read_int(ITEM_TEMPLATE_TABLE,mytemplate,&sprite_offset_y,ITEM_OFFSET_Y,NULL);
			free(mytemplate);
		}

		item = item_list_add(&item_list);

		anim = imageDB_get_anim(ctx,sprite_name);
		free(sprite_name);

		temp_x = map_t2p_x(x,y,default_layer);
		temp_y = map_t2p_y(x,y,default_layer);
		x = temp_x;
		y = temp_y;
		/* Align on tile */
		if( sprite_align == ALIGN_CENTER ) {
			x -= ((anim->w*default_layer->map_zoom)-default_layer->tile_width)/2;
			y -= ((anim->h*default_layer->map_zoom)-default_layer->tile_height)/2;
		}
		if( sprite_align == ALIGN_LOWER ) {
			x -= ((anim->w*default_layer->map_zoom)-default_layer->tile_width)/2;
			y -= (anim->h*default_layer->map_zoom)-default_layer->tile_height;
		}

		y += sprite_offset_y;

		item_set_pos(item,x,y);
		item_set_anim(item,anim,0);
		item_set_zoom_x(item, default_layer->map_zoom );
		item_set_zoom_y(item, default_layer->map_zoom );
		if(font) {
			quantity = resource_get_quantity(item_id[i]);
			sprintf(buf,"%d",quantity);
			item_set_string(item,buf);
			item_set_font(item,font);
		}

		i++;
	}

	deep_free(item_id);
}

/**************************************
**************************************/
static void cb_select_map(void *arg)
{
	item_t * item = (item_t*)arg;
	context_t * ctx = context_get_player();
	char x[SMALL_BUF];
	char y[SMALL_BUF];

	sprintf(x,"%d",item->user1);
	sprintf(y,"%d",item->user2);

	network_send_action(ctx,option->action_select_tile,ctx->map,x,y,NULL);
}

/**************************************
**************************************/
static void cb_redo_map(void *arg)
{
	char * script = NULL;
	char * last_action = NULL;

	cb_select_map(arg);

	last_action = ui_play_get_last_action();
	if(last_action) {
		script = strdup(last_action);
		ui_play_cb_action(script);
		free(script);
	}
}

/**************************************
**************************************/
static void cb_over(void *arg,int x,int y)
{
	item_t * item = (item_t*)arg;

	current_map_x = item->user1;
	current_map_y = item->user2;
}

/**********************************
Set sdl_item item for mouse button callback
**********************************/
static void compose_map_button(context_t * ctx)
{
	int x = 0;
	int y = 0;
	item_t * item;
	anim_t * anim = NULL;

	if ( option && option->cursor_over_tile ) {
		anim = imageDB_get_anim(ctx,option->cursor_over_tile);
	}

	for( y=0 ; y < default_layer->map_h ; y++ ) {
		for ( x=0 ; x < default_layer->map_w ; x++ ) {
			item = item_list_add(&item_list);
			item_set_anim_shape(item,map_t2p_x(x,y,default_layer),map_t2p_y(x,y,default_layer),default_layer->tile_width,default_layer->tile_height);
			item_set_user(item,x,y);
			item_set_click_left(item,cb_select_map,item,NULL);
			item_set_click_right(item,cb_redo_map,item,NULL);
			item_set_over(item,cb_over,item,NULL);
			item_set_anim_over(item,anim,0);
		}
	}
}

/**********************************
Draw the "set" keyword of a layer
**********************************/
static void compose_map_set(context_t * ctx, int layer_index)
{
	int i = 0;
	int x = 0;
	int y = 0;
	anim_t * anim;
	item_t * item;
	char ** tile_set = NULL;
	char layer_name[SMALL_BUF];
	layer_t * layer;

	sprintf(layer_name,"%s%d",MAP_KEY_LAYER,layer_index);
	if(entry_read_list(MAP_TABLE, ctx->map, &tile_set,layer_name,MAP_KEY_SET,NULL) == RET_NOK ) {
		return;
	}

	layer = map_layer_new(ctx->map,layer_index,default_layer);

	while(tile_set[i] != NULL ) {
		/* Skip empty tile */
		if( tile_set[i][0] != 0 ) {
			item = item_list_add(&item_list);
			anim = imageDB_get_anim(ctx,tile_set[i]);
			item_set_pos(item,map_t2p_x(x,y,layer),map_t2p_y(x,y,layer));
			item_set_anim(item,anim,0);
		}

		x++;
		if(x>=layer->map_w) {
			x=0;
			y++;
		}
		i++;
	}

	deep_free(tile_set);

	map_layer_delete(layer);
}

/**********************************
Draw the "list" keyword of a layer
**********************************/
static void compose_map_scenery(context_t * ctx, int layer_index)
{
	int i = 0;
	int x = 0;
	int y = 0;
	char * image_name = NULL;
	anim_t * anim;
	item_t * item;
	char ** scenery_list = NULL;
	char layer_name[SMALL_BUF];

	sprintf(layer_name,"%s%d",MAP_KEY_LAYER,layer_index);
	if(entry_get_group_list(MAP_TABLE, ctx->map, &scenery_list,layer_name,MAP_KEY_SCENERY,NULL) == RET_NOK ) {
		return;
	}

	while(scenery_list[i] != NULL ) {
		if(entry_read_int(MAP_TABLE, ctx->map, &x,layer_name,MAP_KEY_SCENERY,scenery_list[i],MAP_KEY_SCENERY_X,NULL) == RET_NOK ) {
			i++;
			continue;
		}
		if(entry_read_int(MAP_TABLE, ctx->map, &y,layer_name,MAP_KEY_SCENERY,scenery_list[i],MAP_KEY_SCENERY_Y,NULL) == RET_NOK ) {
			i++;
			continue;
		}
		if(entry_read_string(MAP_TABLE, ctx->map, &image_name,layer_name,MAP_KEY_SCENERY,scenery_list[i],MAP_KEY_SCENERY_IMAGE,NULL) == RET_NOK ) {
			i++;
			continue;
		}

		anim = imageDB_get_anim(ctx,image_name);

		item = item_list_add(&item_list);
		item_set_pos(item, x, y);
		item_set_anim(item,anim,0);
		//item_set_anim(item, x*ctx->tile_width, y*ctx->tile_height, anim,0);

		i++;
	}

	deep_free(scenery_list);
}

/**********************************
Show tiles types
**********************************/
static void compose_type(context_t * ctx,int layer_index)
{
	int x = 0;
	int y = 0;
	item_t * item;
	char * type;
	static TTF_Font * font = NULL;
	int w;
	int h;
	char layer_name[SMALL_BUF];

	if( option->show_tile_type == false) {
		return;
	}

	sprintf(layer_name,"%s%d",MAP_KEY_LAYER,layer_index);
	if( entry_exist(MAP_TABLE, ctx->map, layer_name,MAP_KEY_TYPE,NULL) == RET_NOK ) {
		return;
	}

	font = font_get(ctx,ITEM_FONT, ITEM_FONT_SIZE);

	for( x=0; x<default_layer->map_w; x++) {
		for( y=0; y<default_layer->map_h; y++) {
			if(entry_read_list_index(MAP_TABLE,ctx->map,&type,x + y * default_layer->map_w,layer_name,MAP_KEY_TYPE,NULL) == RET_NOK ) {
				continue;
			}

			if( type[0] == 0 ) {
				continue;
			}

			item = item_list_add(&item_list);

			item_set_string(item,type);
			item_set_font(item,font);
			sdl_get_string_size(item->font,item->string,&w,&h);
			item_set_anim_shape(item,map_t2p_x(x,y,default_layer),map_t2p_y(x,y,default_layer),w,h);
		}
	}
}

/**********************************
Compose select cursor
**********************************/
static void compose_select(context_t * ctx)
{
	item_t * item;
	anim_t * anim;
	int pos_tx;
	int pos_ty;
	int x;
	int y;
	context_t * selected_context = NULL;

	/* Tile selection */
	if( option && option->cursor_tile ) {
		if( ctx->selection.map[0] != 0) {
			if( !strcmp(ctx->selection.map, ctx->map) ) {
				pos_tx = ctx->selection.map_coord[0];
				pos_ty = ctx->selection.map_coord[1];

				if( pos_tx != -1 && pos_ty != -1) {
					anim = imageDB_get_anim(ctx,option->cursor_tile);

					item = item_list_add(&item_list);

					/* get pixel coordinate from tile coordinate */
					x = map_t2p_x(pos_tx,pos_ty,default_layer);
					y = map_t2p_y(pos_tx,pos_ty,default_layer);

					item_set_pos(item,x,y);
					item_set_anim(item,anim,0);
				}
			}
		}
	}

	/* Sprite selection */
	if( option && option->cursor_sprite ) {
		if( ctx->selection.id[0] != 0) {
			selected_context = context_find(ctx->selection.id);
			if( selected_context == NULL ) {
				return;
			}

			set_up_sprite(selected_context, option->cursor_sprite);
		}
	}
}

/**********************************
**********************************/
void scr_play_frame_start(context_t * context)
{
}

/**********************************
Compose the character select screen
**********************************/
item_t * scr_play_compose(context_t * ctx)
{
	int bg_red = 0;
	int bg_blue = 0;
	int bg_green = 0;
	char * map_filename;
	int layer_index = 0;
	char * old_sfx = NULL;

	option = option_get();

	if(item_list) {
		item_list_free(item_list);
		item_list = NULL;
	}

	if(ctx->map == NULL ) {
		if(context_update_from_file(ctx) == RET_NOK) {
			return NULL;
		}
	}

	if(init) {
		/* Register this character to receive server notifications */
		network_request_start(ctx,ctx->id);
		ui_play_init();
		init = false;
	}

	sdl_free_keycb();
	sdl_free_mousecb();
	sdl_add_mousecb(MOUSE_WHEEL_UP,cb_zoom);
	sdl_add_mousecb(MOUSE_WHEEL_DOWN,cb_unzoom);

	change_map = ctx->change_map;

	if( change_map == true ) {
		map_filename = strconcat( MAP_TABLE,"/",ctx->map,NULL);
		network_send_req_file(ctx,map_filename);
		free(map_filename);
		if(default_layer) {
			map_layer_delete(default_layer);
		}
		default_layer = map_layer_new(ctx->map,DEFAULT_LAYER,NULL);
	}

	if( default_layer && default_layer->active ) { // Make sure map data are available
		for(layer_index = 0; layer_index < MAX_LAYER; layer_index++) {
			compose_map_set(ctx,layer_index);
			compose_map_scenery(ctx,layer_index);
			compose_item(ctx,layer_index);
			compose_sprite(ctx,layer_index);
			compose_type(ctx,layer_index);
		}
		compose_map_button(ctx);
		compose_select(ctx);

		ui_play_compose(ctx,item_list);

		// force virtual coordinate on map change
		if( change_map == true ) {
			sdl_force_virtual_x(map_t2p_x(ctx->pos_tx,ctx->pos_ty,default_layer) + default_layer->col_width[ctx->pos_tx%default_layer->col_num]/2 + default_layer->row_width[ctx->pos_ty%default_layer->row_num]/2 );
			sdl_force_virtual_y(map_t2p_y(ctx->pos_tx,ctx->pos_ty,default_layer) + default_layer->col_height[ctx->pos_tx%default_layer->col_num]/2 + default_layer->row_height[ctx->pos_ty%default_layer->row_num]/2 );
		}
		// set virtual coordinate on the same map
		else {
			sdl_set_virtual_x(map_t2p_x(ctx->pos_tx,ctx->pos_ty,default_layer) + default_layer->col_width[ctx->pos_tx%default_layer->col_num]/2 + default_layer->row_width[ctx->pos_ty%default_layer->row_num]/2 );
			sdl_set_virtual_y(map_t2p_y(ctx->pos_tx,ctx->pos_ty,default_layer) + default_layer->col_height[ctx->pos_tx%default_layer->col_num]/2 + default_layer->row_height[ctx->pos_ty%default_layer->row_num]/2 );
		}
	}

	entry_read_int(MAP_TABLE,ctx->map,&bg_red,MAP_KEY_BG_RED,NULL);
	entry_read_int(MAP_TABLE,ctx->map,&bg_blue,MAP_KEY_BG_BLUE,NULL);
	entry_read_int(MAP_TABLE,ctx->map,&bg_green,MAP_KEY_BG_GREEN,NULL);
	SDL_SetRenderDrawColor(ctx->render, bg_red, bg_blue, bg_green, 255);

	old_sfx = sfx;
	sfx = NULL;

	entry_read_string(MAP_TABLE,ctx->map,&sfx,MAP_SFX,NULL);

	if(old_sfx)  {
		if( sfx ) {
			if( strcmp(old_sfx,sfx) ) {
				sfx_stop(ctx,old_sfx);
			}
		} else  { // sfx == NULL
			sfx_stop(ctx,old_sfx);
		}
		free(old_sfx);
	}

	if( sfx && sfx[0]!=0 ) {
		sfx_play(ctx,sfx,NO_RESTART);
	}

	return item_list;
}
