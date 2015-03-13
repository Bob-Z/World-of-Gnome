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

#define ITEM_FONT "/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-C.ttf"
#define ITEM_FONT_SIZE 15

#define NORTH (1<<0)
#define SOUTH (1<<1)
#define EAST (1<<2)
#define WEST (1<<3)

#define ALIGN_CENTER	0
#define ALIGN_LOWER	1

#define MAX_COL		16
#define MAX_ROW		16

// Convert tiles coordinates into pixels coordinates
#define T2P_X(x,y)  (x * col_width[x%col_num] + y * row_width[y%row_num])
#define T2P_Y(x,y)  (x * col_height[x%col_num] + y * row_height[y%row_num])

static item_t * item_list = NULL;
static int change_map = 0;
static int init = true;
static int current_map_x = -1;
static int current_map_y = -1;
static option_t * option;
static int tile_width = -1;
static int tile_height = -1;
static int map_w = -1;
static int map_h = -1;
static int use_next = false;
static int col_width[MAX_COL];
static int col_height[MAX_COL];
static int col_num = 0;
static int row_width[MAX_ROW];
static int row_height[MAX_ROW];
static int row_num = 0;
static int sprite_align = ALIGN_CENTER;
static int sprite_offset_y = 0;
static double map_zoom = 0.0;

/**********************************
**********************************/
void scr_play_init(int init_value)
{
	init = init_value;
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

	cb_select_sprite(arg);

	script = strdup(ui_play_get_last_action());
	ui_play_cb_action(script);
	free(script);
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
Draw a single sprite
if image_file_name is not NULL, this file is used as an image rather than the normal sprite image
**********************************/
static void draw_sprite(context_t * ctx, const char * image_file_name)
{
	char * sprite_name = NULL;
	anim_t * sprite;
	item_t * item;
	int x;
	int y;
	int ox;
	int oy;
	Uint32 current_time;
	int angle;
	int flip;
	int force_flip;
	int move_status;
	char * zoom_str = NULL;
	double zoom = 1.0;

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

	if( image_file_name ) {
		sprite = imageDB_get_anim(player_context,image_file_name);
	} else {
		if(!entry_read_string(CHARACTER_TABLE,ctx->id,&sprite_name,CHARACTER_KEY_SPRITE,NULL)) {
			werr(LOGDEV,"Can't read sprite name for \"%s\"",ctx->id);
			return;
		}

		sprite = imageDB_get_anim(player_context,sprite_name);
		free(sprite_name);
	}

	item = item_list_add(&item_list);

	current_time = SDL_GetTicks();

	/* Force position when the player has changed map */
	if(change_map) {
		ctx->pos_tick = 0;
	}
	/* Force position when this context has changed map */
	if(ctx->change_map) {
		ctx->pos_tick = 0;
		ctx->change_map = 0;
	}

	if( ctx->pos_tick == 0 ) {
		ctx->cur_pos_x = ctx->pos_x;
		ctx->cur_pos_y = ctx->pos_y;
		ctx->pos_tick = 1;
	}

	/* If previous animation has ended */
	if( ctx->pos_tick + VIRTUAL_ANIM_DURATION < current_time ) {
		ctx->old_pos_x = ctx->cur_pos_x;
		ctx->old_pos_y = ctx->cur_pos_y;
	}

	/* Detect sprite movement, initiate animation */
	if(ctx->pos_x != ctx->cur_pos_x||ctx->pos_y != ctx->cur_pos_y) {
		ctx->pos_tick = current_time;

		/* flip need to remember previous direction to avoid resetting a
		   east -> west flip when a sprite goes to north for instance.
		   On the contrary rotation must not remember previous state, or
		   the rotation will be wrong.
		   Hence the distinction between orientation (no memory) and
		   direction (memory). */
		ctx->orientation = 0;
		/* Compute direction */
		if( ctx->pos_x > ctx->cur_pos_x ) {
			ctx->direction &= ~WEST;
			ctx->direction |= EAST;
			ctx->orientation |= EAST;
		}
		if( ctx->pos_x < ctx->cur_pos_x ) {
			ctx->direction &= ~EAST;
			ctx->direction |= WEST;
			ctx->orientation |= WEST;
		}
		if( ctx->pos_y > ctx->cur_pos_y ) {
			ctx->direction &= ~NORTH;
			ctx->direction |= SOUTH;
			ctx->orientation |= SOUTH;
		}
		if( ctx->pos_y < ctx->cur_pos_y ) {
			ctx->direction &= ~SOUTH;
			ctx->direction |= NORTH;
			ctx->orientation |= NORTH;
		}

		ctx->old_pos_x = ctx->cur_pos_x;
		ctx->old_pos_y = ctx->cur_pos_y;
		ctx->cur_pos_x = ctx->pos_x;
		ctx->cur_pos_y = ctx->pos_y;
	}

	/* Get position in pixel */
	x = T2P_X(ctx->cur_pos_x,ctx->cur_pos_y);
	y = T2P_Y(ctx->cur_pos_x,ctx->cur_pos_y);
	ox = T2P_X(ctx->old_pos_x,ctx->old_pos_y);
	oy = T2P_Y(ctx->old_pos_x,ctx->old_pos_y);

	/* Get per sprite zoom */
	if(entry_read_string(CHARACTER_TABLE,ctx->id,&zoom_str,CHARACTER_KEY_ZOOM,NULL)) {
		zoom = atof(zoom_str);
		free(zoom_str);
	}

	/* Align sprite on tile */
	if( sprite_align == ALIGN_CENTER ) {
		x -= ((sprite->w*map_zoom*zoom)-tile_width)/2;
		y -= ((sprite->h*map_zoom*zoom)-tile_height)/2;
		ox -= ((sprite->w*map_zoom*zoom)-tile_width)/2;
		oy -= ((sprite->h*map_zoom*zoom)-tile_height)/2;
	}
	if( sprite_align == ALIGN_LOWER ) {
		x -= ((sprite->w*map_zoom*zoom)-tile_width)/2;
		y -= (sprite->h*map_zoom*zoom)-tile_height ;
		ox -= ((sprite->w*map_zoom*zoom)-tile_width)/2;
		oy -= (sprite->h*map_zoom*zoom)-tile_height;
	}

	y += sprite_offset_y;
	oy += sprite_offset_y;

	item_set_smooth_anim(item,x,y,ox,oy,ctx->pos_tick,sprite);

	/* Get rotation configuration */
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

	/* Get flip configuration */
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

	item_set_zoom_x(item,zoom * map_zoom );
	item_set_zoom_y(item,zoom * map_zoom );
}

/**********************************
Compose sprites
**********************************/
static void compose_sprite(context_t * ctx)
{
	context_lock_list();

	while(ctx != NULL ) {
		draw_sprite(ctx,NULL);

		ctx = ctx->next;
	}

	context_unlock_list();
}

/**********************************
Compose item on map
**********************************/
static void compose_item(context_t * ctx)
{
	char * sprite_name = NULL;
	anim_t * anim;
	item_t * item;
	int x;
	int y;
	int temp_x;
	int temp_y;
	char ** item_id;
	int i;
	static TTF_Font * font = NULL;
	char * template;
	int quantity;
	char buf[1024];

	if ( font == NULL ) {
		font = TTF_OpenFont(ITEM_FONT, ITEM_FONT_SIZE);
	}

	if(!entry_get_group_list(MAP_TABLE,ctx->map,&item_id,MAP_ENTRY_ITEM_LIST,NULL)) {
		return;
	}

	i=0;
	while( item_id[i] != NULL ) {
		if(!entry_read_int(MAP_TABLE,ctx->map,&x,MAP_ENTRY_ITEM_LIST,item_id[i],MAP_ITEM_POS_X,NULL)) {
			i++;
			continue;
		}

		if(!entry_read_int(MAP_TABLE,ctx->map,&y,MAP_ENTRY_ITEM_LIST,item_id[i],MAP_ITEM_POS_Y,NULL)) {
			i++;
			continue;
		}

		template = item_is_resource(item_id[i]);

		if ( template == NULL ) {
			if(!entry_read_string(ITEM_TABLE,item_id[i],&sprite_name,ITEM_SPRITE,NULL)) {
				i++;
				continue;
			}
		} else {
			if(!entry_read_string(ITEM_TEMPLATE_TABLE,template,&sprite_name,ITEM_SPRITE,NULL)) {
				free(template);
				i++;
				continue;
			}
			free(template);
		}

		item = item_list_add(&item_list);

		anim = imageDB_get_anim(ctx,sprite_name);
		free(sprite_name);

		temp_x = T2P_X(x,y);
		temp_y = T2P_Y(x,y);
		x = temp_x;
		y = temp_y;
		/* Align on tile */
		if( sprite_align == ALIGN_CENTER ) {
			x -= ((anim->w*map_zoom)-tile_width)/2;
			y -= ((anim->h*map_zoom)-tile_height)/2;
		}
		if( sprite_align == ALIGN_LOWER ) {
			x -= ((anim->w*map_zoom)-tile_width)/2;
			y -= (anim->h*map_zoom)-tile_height;
		}

		y += sprite_offset_y;

		item_set_anim(item,x,y,anim);
		item_set_zoom_x(item, map_zoom );
		item_set_zoom_y(item, map_zoom );
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

	sprintf(x,"%d",item->tile_x);
	sprintf(y,"%d",item->tile_y);

	network_send_action(ctx,option->action_select_tile,ctx->map,x,y,NULL);
}

/**************************************
**************************************/
static void cb_redo_map(void *arg)
{
	char * script = NULL;

	cb_select_map(arg);

	script = strdup(ui_play_get_last_action());
	ui_play_cb_action(script);
	free(script);
}

/**************************************
**************************************/
static void cb_over(void *arg,int x,int y)
{
	item_t * item = (item_t*)arg;

	current_map_x = item->tile_x;
	current_map_y = item->tile_y;
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

	for( y=0 ; y < map_h ; y++ ) {
		for ( x=0 ; x < map_w ; x++ ) {
			item = item_list_add(&item_list);
			item_set_frame_shape(item,T2P_X(x,y),T2P_Y(x,y),tile_width,tile_height);
			item_set_tile(item,x,y);
			item_set_click_left(item,cb_select_map,item,NULL);
			item_set_click_right(item,cb_redo_map,item,NULL);
			item_set_over(item,cb_over,item,NULL);
			item_set_anim_over(item,anim);
		}
	}
}

/**********************************
Draw a map from a "set" keyword
return 1 if a map has been drawn
**********************************/
static int compose_map_set(context_t * ctx, int level)
{
	int i = 0;
	int x = 0;
	int y = 0;
	int _map_w = map_w;
	int _map_h = map_h;
	int _tile_width = tile_width;
	int _tile_height = tile_height;


	anim_t * anim;
	item_t * item;
	char buf[SMALL_BUF];
	char ** tile_set = NULL;

	sprintf(buf,"%s%d",MAP_KEY_SET,level);
	if(!entry_read_list(MAP_TABLE, ctx->map, &tile_set,buf,NULL)) {
		return 0;
	}
	sprintf(buf,"%s%d",MAP_KEY_WIDTH,level);
	entry_read_int(MAP_TABLE, ctx->map, &_map_w,buf,NULL);
	sprintf(buf,"%s%d",MAP_KEY_HEIGHT,level);
	entry_read_int(MAP_TABLE, ctx->map, &_map_h,buf,NULL);

	sprintf(buf,"%s%d",MAP_KEY_TILE_WIDTH,level);
	entry_read_int(MAP_TABLE, ctx->map, &_tile_width,buf,NULL);
	sprintf(buf,"%s%d",MAP_KEY_TILE_HEIGHT,level);
	entry_read_int(MAP_TABLE, ctx->map, &_tile_height,buf,NULL);

	while(tile_set[i] != NULL ) {
		/* Skip empty tile */
		if( tile_set[i][0] != 0 ) {
			item = item_list_add(&item_list);
			anim = imageDB_get_anim(ctx,tile_set[i]);
			item_set_anim(item,T2P_X(x,y),T2P_Y(x,y),anim);
		}

		x++;
		if(x>=_map_w) {
			x=0;
			y++;
		}
		i++;
	}

	deep_free(tile_set);
	return 1;
}

/**********************************
Draw a map from a "list" keyword
return 1 if a map has been drawn
**********************************/
static int compose_map_list(context_t * ctx, int level)
{
	int i = 0;
	int x = 0;
	int y = 0;
	anim_t * anim;
	item_t * item;
	char buf[SMALL_BUF];
	char ** tile_list = NULL;

	sprintf(buf,"%s%d",MAP_KEY_LIST,level);
	if(!entry_read_list(MAP_TABLE, ctx->map, &tile_list,buf,NULL)) {
		return 0;
	}

	while(tile_list[i] != NULL ) {
		x = atoi(tile_list[i]);
		i++;
		y = atoi(tile_list[i]);
		i++;

		anim = imageDB_get_anim(ctx,tile_list[i]);

		item = item_list_add(&item_list);
		item_set_anim(item, x, y, anim);
		//item_set_anim(item, x*ctx->tile_width, y*ctx->tile_height, anim);

		i++;
	}

	deep_free(tile_list);
	return 1;
}

/**********************************
Compose the map
**********************************/
static void compose_map(context_t * ctx)
{
	int sprite_level = 0;
	int current_level = 0;
	int map_drawn = 0;
	int sprite_drawn = 0;
	entry_read_int(MAP_TABLE, ctx->map, &sprite_level,MAP_KEY_SPRITE_LEVEL,NULL);

	/* Draw all maps */
	while(TRUE) {
		/* Draw sprite */
		if ( !sprite_drawn && sprite_level < current_level ) {
			compose_item(ctx);
			compose_sprite(ctx);
			sprite_drawn = 1;
		}

		map_drawn = 0;
		map_drawn |= compose_map_set(ctx,current_level);
		map_drawn |= compose_map_list(ctx,current_level);

		if( map_drawn == 0 ) {
			break;
		}

		current_level++;
	}

	/* Set mouse callback */
	compose_map_button(ctx);

}

/**********************************
Show tiles types
**********************************/
static void compose_type(context_t * ctx)
{
	int x = 0;
	int y = 0;
	item_t * item;
	char * type;
	static TTF_Font * font = NULL;
	int w;
	int h;

	if( option->show_tile_type == false) {
		return;
	}

	if( font == NULL ) {
		font = TTF_OpenFont(ITEM_FONT, ITEM_FONT_SIZE);
	}
	if( font == NULL ) {
		return;
	}

	for( x=0; x<map_w; x++) {
		for( y=0; y<map_h; y++) {
			if(!entry_read_list_index(MAP_TABLE,ctx->map,&type,x + y * map_w,MAP_KEY_TYPE,NULL)) {
				continue;
			}

			if( type[0] == 0 ) {
				continue;
			}

			item = item_list_add(&item_list);

			item_set_string(item,type);
			item_set_font(item,font);
			sdl_get_string_size(item->font,item->string,&w,&h);
			item_set_frame_shape(item,T2P_X(x,y),T2P_Y(x,y),w,h);
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
	int pos_x;
	int pos_y;
	int x;
	int y;
	context_t * selected_context = NULL;

	/* Tile selection */
	if( option && option->cursor_tile ) {
		if( ctx->selection.map[0] != 0) {
			if( !strcmp(ctx->selection.map, ctx->map) ) {
				pos_x = ctx->selection.map_coord[0];
				pos_y = ctx->selection.map_coord[1];

				if( pos_x != -1 && pos_y != -1) {
					anim = imageDB_get_anim(ctx,option->cursor_tile);

					item = item_list_add(&item_list);

					/* get pixel coordinate from tile coordinate */
					x = T2P_X(pos_x,pos_y);
					y = T2P_Y(pos_x,pos_y);

					/* Center on tile */
					x -= (anim->w-tile_width)/2;
					y -= (anim->h-tile_height)/2;

					item_set_anim(item,x,y,anim);
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

			draw_sprite(selected_context, option->cursor_sprite);
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
	char * zoom_str;

	option = option_get();

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
		network_request_start(ctx,ctx->id);
		ui_play_init();
		init = false;
	}

	sdl_free_keycb();
	sdl_free_mousecb();
	sdl_add_mousecb(MOUSE_WHEEL_UP,cb_zoom);
	sdl_add_mousecb(MOUSE_WHEEL_DOWN,cb_unzoom);

	change_map = ctx->change_map;
	/* Check if map has changed */
	if( change_map ) {
		map_filename = strconcat( MAP_TABLE,"/",ctx->map,NULL);
		network_send_req_file(ctx,map_filename);
		free(map_filename);
                entry_read_int(MAP_TABLE, ctx->map, &map_w,MAP_KEY_WIDTH,NULL);
                entry_read_int(MAP_TABLE, ctx->map, &map_h,MAP_KEY_HEIGHT,NULL);
                entry_read_int(MAP_TABLE, ctx->map, &tile_width,MAP_KEY_TILE_WIDTH,NULL);
                entry_read_int(MAP_TABLE, ctx->map, &tile_height,MAP_KEY_TILE_HEIGHT,NULL);
                entry_read_int(MAP_TABLE, ctx->map, &sprite_align,MAP_KEY_SPRITE_ALIGN,NULL);
                entry_read_int(MAP_TABLE, ctx->map, &sprite_offset_y,MAP_KEY_SPRITE_OFFSET_Y,NULL);
		map_zoom = 1.0;
		if(entry_read_string(MAP_TABLE,ctx->map,&zoom_str,MAP_KEY_SPRITE_ZOOM,NULL)) {
			map_zoom = atof(zoom_str);
			free(zoom_str);
		}

		/* Automatic tiling */
		col_num = 1;
		col_width[0] = tile_width;
		col_height[0] = 0;
		row_num = 1;
		row_width[0] = 0;
		row_height[0] = tile_height;

		/* Custom tiling */
		entry_read_int(MAP_TABLE, ctx->map, &col_width[0],MAP_KEY_COL_WIDTH,NULL);
		entry_read_int(MAP_TABLE, ctx->map, &col_height[0],MAP_KEY_COL_HEIGHT,NULL);
		entry_read_int(MAP_TABLE, ctx->map, &row_width[0],MAP_KEY_ROW_WIDTH,NULL);
		entry_read_int(MAP_TABLE, ctx->map, &row_height[0],MAP_KEY_ROW_HEIGHT,NULL);
	}

	compose_map(ctx);
	compose_type(ctx);
	compose_select(ctx);

	ui_play_compose(ctx,item_list);

	/* force virtual coordinate on map change */
	if(change_map) {
		sdl_force_virtual_x(T2P_X(ctx->pos_x,ctx->pos_y) + col_width[ctx->pos_x%col_num]/2 + row_width[ctx->pos_y%row_num]/2 );
		sdl_force_virtual_y(T2P_Y(ctx->pos_x,ctx->pos_y) + col_height[ctx->pos_x%col_num]/2 + row_height[ctx->pos_y%row_num]/2 );
	}
	/* set virtual coordinate on the same map */
	else {
		sdl_set_virtual_x(T2P_X(ctx->pos_x,ctx->pos_y) + col_width[ctx->pos_x%col_num]/2 + row_width[ctx->pos_y%row_num]/2 );
		sdl_set_virtual_y(T2P_Y(ctx->pos_x,ctx->pos_y) + col_height[ctx->pos_x%col_num]/2 + row_height[ctx->pos_y%row_num]/2 );
	}

	entry_read_int(MAP_TABLE,ctx->map,&bg_red,MAP_KEY_BG_RED,NULL);
	entry_read_int(MAP_TABLE,ctx->map,&bg_blue,MAP_KEY_BG_BLUE,NULL);
	entry_read_int(MAP_TABLE,ctx->map,&bg_green,MAP_KEY_BG_GREEN,NULL);
	SDL_SetRenderDrawColor(ctx->render, bg_red, bg_blue, bg_green, 255);

	return item_list;
}
