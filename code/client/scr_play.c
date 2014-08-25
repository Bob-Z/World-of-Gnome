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

#define FONT "/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-C.ttf"
#define FONT_SIZE 30
#define TEXT_FONT "/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-C.ttf"
#define TEXT_FONT_SIZE 15
#define TEXT_TIMEOUT 5000 /* Text display timeout */
#define ITEM_FONT "/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-C.ttf"
#define ITEM_FONT_SIZE 15

#define NORTH (1<<0)
#define SOUTH (1<<1)
#define EAST (1<<2)
#define WEST (1<<3)

static char ** attribute_string = NULL;
static char text_buffer[2048];

static item_t * item_list = NULL;
static int change_map = 0;

static int action_bar_height;
static int attribute_height;
static char * last_action_script = NULL;

static void cb_action(void * arg);

/****************************
Keyboard callback
****************************/
static void cb_quit(void * arg)
{
	screen_set_screen(SCREEN_SELECT);
}

/****************************
****************************/
static void key_up(void * arg)
{
	context_t * ctx = context_get_list_first();

	network_send_action(ctx,"move_up.lua",NULL);
}

/**************************************
**************************************/
static void key_down(void * arg)
{
	context_t * ctx = context_get_list_first();

	network_send_action(ctx,"move_down.lua",NULL);
}

/**************************************
**************************************/
static void key_left(void * arg)
{
	context_t * ctx = context_get_list_first();

	network_send_action(ctx,"move_left.lua",NULL);
}

/**************************************
**************************************/
static void key_right(void * arg)
{
	context_t * ctx = context_get_list_first();

	network_send_action(ctx,"move_right.lua",NULL);
}

/**************************************
**************************************/
static void show_inventory(void * arg)
{
	screen_set_screen(SCREEN_INVENTORY);
}

/**************************************
**************************************/
static void keyboard_text(void * arg)
{
	char * text = (char*)arg;

	network_send_action(context_get_list_first(),WOG_CHAT,text,NULL);
	text_buffer[0]=0;
	screen_compose();
}

/**************************************
**************************************/
static void cb_select_map(void *arg)
{
	item_t * item = (item_t*)arg;
	context_t * ctx = context_get_list_first();

	ctx->selection.map_coord[0]= item->tile_x;
	ctx->selection.map_coord[1]= item->tile_y;
	network_send_context(ctx);
}

/**************************************
**************************************/
static void cb_redo_map(void *arg)
{
	char * script = NULL;

	cb_select_map(arg);

	script = strdup(last_action_script);
	cb_action(script);
	free(script);
}

/**********************************
Compose the map
**********************************/
static void compose_map(context_t * ctx)
{
	int i;
	char ** value = NULL;
	int x = 0;
	int y = 0;
	char * tile_image = NULL;
	anim_t * anim;
	item_t * item;

	if( ctx->map_w == -1 ) {
		if(!entry_read_int(MAP_TABLE, ctx->map, &i,MAP_KEY_SIZE_X,NULL)) {
			return;
		}
		context_set_map_w( ctx, i);
	}
	if( ctx->map_h == -1 ) {
		if(!entry_read_int(MAP_TABLE, ctx->map, &i,MAP_KEY_SIZE_Y,NULL)) {
			return;
		}
		context_set_map_h( ctx, i);
	}
	if( ctx->tile_x == -1 ) {
		if(!entry_read_int(MAP_TABLE, ctx->map, &i,MAP_KEY_TILE_SIZE_X,NULL)) {
			return;
		}
		context_set_tile_x( ctx, i);
	}
	if( ctx->tile_x == -1 ) {
		if(!entry_read_int(MAP_TABLE, ctx->map, &i,MAP_KEY_TILE_SIZE_X,NULL)) {
			return;
		}
		context_set_tile_x( ctx, i);
	}
	if( ctx->tile_y == -1 ) {
		if(!entry_read_int(MAP_TABLE, ctx->map,&i,MAP_KEY_TILE_SIZE_Y,NULL)) {
			return;
		}
		context_set_tile_y( ctx, i);
	}
	if(!entry_read_list(MAP_TABLE, ctx->map, &value,MAP_KEY_SET,NULL)) {
		return;
	}

	/* Parse map string */
	i=0;
	while(value[i] != NULL ) {
		if(entry_read_string(TILE_TABLE,value[i],&tile_image,TILE_KEY_IMAGE,NULL)) {
#if 0
			/* Save description for caller */
			entry_read_string(TILE_TABLE,tile_name,description,TILE_KEY_TEXT,NULL);
			free(description);
#endif
			item = item_list_add(&item_list);

			anim = imageDB_get_anim(ctx,tile_image);
			free(tile_image);

			item_set_anim(item,x*ctx->tile_x,y*ctx->tile_y,anim);
			item_set_tile(item,x,y);
			item_set_click_left(item,cb_select_map,item,NULL);
			item_set_click_right(item,cb_redo_map,item,NULL);
		}

		x++;
		if(x>=ctx->map_w) {
			x=0;
			y++;
		}
		i++;
	}

	deep_free(value);
}

/**********************************
**********************************/
static void cb_select_sprite(void *arg)
{
	char * id = (char*)arg;

	context_t * ctx = context_get_list_first();
	ctx->selection.id= id;
	network_send_context(ctx);
}

/**********************************
**********************************/
static void cb_redo_sprite(void *arg)
{
	char * script = NULL;

	cb_select_sprite(arg);

	script = strdup(last_action_script);
	cb_action(last_action_script);
	free(script);
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

		x = x*ctx->tile_x;
		y = y*ctx->tile_y;
		/* Center sprite on tile */
		x -= (anim->w-ctx->tile_x)/2;
		y -= (anim->h-ctx->tile_y)/2;

		item_set_anim(item,x,y,anim);
		if(font) {
			quantity = item_get_quantity(item_id[i]);
			sprintf(buf,"%d",quantity);
			item_set_string(item,buf);
			item_set_font(item,font);
		}

		i++;
	}

	deep_free(item_id);
}

/**********************************
Compose sprites
**********************************/
static void compose_sprite(context_t * ctx)
{
	char * sprite_name = NULL;
	anim_t * anim;
	item_t * item;
	int x;
	int y;
	int ox;
	int oy;
	Uint32 timer;
	int angle;
	int flip;
	context_t * player_context = context_get_list_first();

	context_lock_list();

	while(ctx != NULL ) {
		if( ctx->map == NULL ) {
			ctx = ctx->next;
			continue;
		}
		if( strcmp(ctx->map,player_context->map)) {
			ctx = ctx->next;
			continue;
		}

		/* compute the sprite file name */
		if(!entry_read_string(CHARACTER_TABLE,ctx->id,&sprite_name,CHARACTER_KEY_SPRITE,NULL)) {
			werr(LOGDEV,"ID=%s. Can't read sprite name for \"%s\" type",ctx->id,ctx->type);
			ctx = ctx->next;
			continue;
		}

		anim = imageDB_get_anim(player_context,sprite_name);
		free(sprite_name);

		item = item_list_add(&item_list);

		timer = SDL_GetTicks();

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
		if( ctx->pos_tick + VIRTUAL_ANIM_DURATION < timer ) {
			ctx->old_pos_x = ctx->cur_pos_x;
			ctx->old_pos_y = ctx->cur_pos_y;
		}

		/* Detect sprite movement, initiate animation */
		if(ctx->pos_x != ctx->cur_pos_x||ctx->pos_y != ctx->cur_pos_y) {
			ctx->pos_tick = timer;

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

			ctx->cur_pos_x = ctx->pos_x;
			ctx->cur_pos_y = ctx->pos_y;
		}

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
		flip = 0;
		if( angle == 0 ) {
			if( ctx->direction & NORTH ) {
				entry_read_int(CHARACTER_TABLE,ctx->id,&flip,CHARACTER_KEY_DIR_N_FLIP,NULL);
			}
			if( ctx->direction & SOUTH ) {
				entry_read_int(CHARACTER_TABLE,ctx->id,&flip,CHARACTER_KEY_DIR_S_FLIP,NULL);
			}
			if( ctx->direction & WEST ) {
				entry_read_int(CHARACTER_TABLE,ctx->id,&flip,CHARACTER_KEY_DIR_W_FLIP,NULL);
			}
			if( ctx->direction & EAST ) {
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
		item_set_click_left(item,cb_select_sprite,ctx->id,NULL);
		item_set_click_right(item,cb_redo_sprite,item,NULL);

		ctx = ctx->next;
	}

	context_unlock_list();
}

/**********************************
Compose attribute
**********************************/
static void compose_attribute(context_t * ctx)
{
	item_t * item;
	char ** name_list;
	int index = 0;
	int value;
	int y = 0;
	int num_attr = 0;
	char buf[1024];
	int w,h;
	static TTF_Font * font = NULL;

	if ( font == NULL ) {
		font = TTF_OpenFont(FONT, FONT_SIZE);
	}
	if ( font == NULL ) {
		return;
	}

	if(attribute_string) {
		index = 0;
		while(attribute_string[index]) {
			free(attribute_string[index]);
			attribute_string[index]=NULL;
			index++;
		}
		free(attribute_string);
		attribute_string=NULL;
	}

	if(!entry_get_group_list(CHARACTER_TABLE,ctx->id,&name_list,ATTRIBUTE_GROUP,NULL) ) {
		return;
	}

	index=0;
	while( name_list[index] != NULL) {
		if(!entry_read_int(CHARACTER_TABLE,ctx->id,&value,ATTRIBUTE_GROUP,name_list[index],ATTRIBUTE_CURRENT,NULL)) {
			index++;
			continue;
		}

		num_attr++;
		attribute_string = realloc(attribute_string, (num_attr+1)*sizeof(char*));
		sprintf(buf,"%s: %d",name_list[index],value);
		attribute_string[num_attr-1] = strdup(buf);
		attribute_string[num_attr]=NULL;

		item = item_list_add(&item_list);

		item_set_overlay(item,1);
		item_set_string(item,attribute_string[num_attr-1]);
		item_set_font(item,font);
		sdl_get_string_size(item->font,item->string,&w,&h);
		item_set_frame_shape(item,0,y,w,h);
		y+=h;
		if(attribute_height<y) {
			attribute_height = y;
		}

		index++;
	}

	deep_free(name_list);
}

/**********************************
**********************************/
static void cb_action(void * arg)
{
	char * script = (char *)arg;

	network_send_action(context_get_list_first(),script,NULL);

	if( last_action_script ) {
		free(last_action_script);
		last_action_script = NULL;
	}

	if( arg ) {
		last_action_script = strdup(arg);
	} else {
		last_action_script = NULL;
	}
}

/**********************************
Compose action icon
**********************************/
static void compose_action(context_t * ctx)
{
	char ** action_list = NULL;
	char * text = NULL;
	char * icon = NULL;
	char * script = NULL;
	anim_t * anim;
	item_t * item;
	int sw = 0;
	int sh = 0;
	int x=0;
	int i;

	SDL_GetRendererOutputSize(ctx->render,&sw,&sh);

	action_bar_height = 0;

	/* Read action list for current user */
	if(!entry_read_list(CHARACTER_TABLE,ctx->id,&action_list,CHARACTER_KEY_ACTION,NULL)) {
		return;
	}

	i=0;
	while(action_list[i] != NULL ) {
		if(text) {
			free(text);
			text = NULL;
		}
		if(icon) {
			free(icon);
			icon = NULL;
		}
		if(script) {
			free(script);
			script = NULL;
		}

		if(!entry_read_string(ACTION_TABLE,action_list[i],&text,ACTION_KEY_TEXT,NULL)) {
			i++;
			continue;
		}
		if(!entry_read_string(ACTION_TABLE,action_list[i],&icon,ACTION_KEY_ICON,NULL)) {
			i++;
			continue;
		}
		if(!entry_read_string(ACTION_TABLE,action_list[i],&script,ACTION_KEY_SCRIPT,NULL)) {
			i++;
			continue;
		}

		/* load image */
		anim = imageDB_get_anim(ctx, icon);

		item = item_list_add(&item_list);

		item_set_overlay(item,1);
		item_set_anim(item,x,sh-anim->h,anim);
		x += anim->w;
		item_set_click_left(item,cb_action,(void*)strdup(script),free);
		if( action_bar_height < anim->h ) {
			action_bar_height = anim->h;
		}

		i++;
	}

	deep_free(action_list);

	if(text) {
		free(text);
		text = NULL;
	}
	if(icon) {
		free(icon);
		icon = NULL;
	}
	if(script) {
		free(script);
		script = NULL;
	}
}

/**********************************
**********************************/
static void cb_select_slot(void * arg)
{
	char * id = (char*)arg;
	context_t * ctx = context_get_list_first();

	ctx->selection.equipment = id;
	network_send_context(ctx);
}

/**********************************
Compose equipment icon
**********************************/
static void compose_equipment(context_t * ctx)
{
	char ** name_list = NULL;
	anim_t * anim;
	anim_t * anim2;
	anim_t * anim3;
	item_t * item;
	int sw = 0;
	int sh = 0;
	int y=0;
	int x=0;
	int h1;
	int index;
	char * template = NULL;
#if 0
	char * name;
#endif
	char * icon_name = NULL;
	char * equipped_name = NULL;
#if 0
	char * equipped_text = NULL;
#endif
	char * equipped_icon_name = NULL;
	char * inventory_icon_name = NULL;
	static anim_t * inventory_icon = NULL;
	int max_h;
	int max_w;

	SDL_GetRendererOutputSize(ctx->render,&sw,&sh);

	entry_get_group_list(CHARACTER_TABLE,ctx->id,&name_list,EQUIPMENT_GROUP,NULL);

	max_w = 0;
	max_h = 0;
	index=0;
	while( name_list && name_list[index] != NULL) {
#if 0
		/* Get the slot name */
		if(!entry_read_string(CHARACTER_TABLE,ctx->id,&item_name,EQUIPMENT_GROUP,name_list[index],EQUIPMENT_NAME,NULL)) {
			name = strdup(name_list[index]);
		} else {
			name = item_name;
		}
		free(item_name);
#endif
		h1 = 0;
		/* Get the slot icon */
		if(!entry_read_string(CHARACTER_TABLE,ctx->id,&icon_name,EQUIPMENT_GROUP,name_list[index],EQUIPMENT_ICON,NULL)) {
			continue;
		} else {
			/* load image */
			anim = imageDB_get_anim(ctx, icon_name);
			free(icon_name);

			item = item_list_add(&item_list);

			x = sw-anim->w;
			h1 = anim->h;
			item_set_overlay(item,1);
			item_set_anim(item,x,y,anim);

			item_set_click_left(item,cb_select_slot,strdup(name_list[index]),NULL);

			if(anim->w > max_w) {
				max_w = anim->w;
			}
			if(anim->h > max_h) {
				max_h = anim->h;
			}
		}

		/* Is there an equipped object ? */
		if(entry_read_string(CHARACTER_TABLE,ctx->id,&equipped_name,EQUIPMENT_GROUP,name_list[index],EQUIPMENT_EQUIPPED,NULL)) {
#if 0
			/* Get the equipped object name */
			if(!entry_read_string(ITEM_TABLE,equipped_name,&equipped_text,ITEM_NAME,NULL)) {
				werr(LOGDEV,"Can't read object %s name in equipment slot %s",equipped_name,name_list[index]);
			}
			free(equipped_text);
#endif
			/* Get it's icon */
			if(!entry_read_string(ITEM_TABLE,equipped_name,&equipped_icon_name,ITEM_ICON,NULL)) {
				werr(LOGDEV,"Can't read object %s icon in equipment slot %s",equipped_name,name_list[index]);
			} else {
				item = item_list_add(&item_list);

				anim2 = imageDB_get_anim(ctx, equipped_icon_name);
				free(equipped_icon_name);

				item_set_overlay(item,1);
				item_set_anim(item,x-anim->w,y,anim2);
				item_set_click_left(item,cb_select_slot,strdup(name_list[index]),NULL);
				if(h1 < anim->h) {
					h1 = anim->h;
				}
			}
			free(equipped_name);
		}

		/* Draw selection cursor */
		if( ctx->selection.equipment != NULL) {
			if( !strcmp(ctx->selection.equipment,name_list[index]) ) {
				anim3 = imageDB_get_anim(ctx,CURSOR_EQUIP_FILE);

				item = item_list_add(&item_list);

				/* Center on icon */
				item_set_overlay(item,1);
				item_set_anim(item,x - (anim3->w-anim->w)/2, y - (anim3->h-anim->w)/2, anim3);
			}
		}

		if(h1 > anim->h) {
			y += h1;
		} else {
			y += anim->h;
		}

		index++;
	}

	/* Draw selected item */
	if( ctx->selection.inventory != NULL) {
		template = item_is_resource(ctx->selection.inventory);

		if ( template == NULL ) {
			if(!entry_read_string(ITEM_TABLE,ctx->selection.inventory,&inventory_icon_name,ITEM_ICON,NULL)) {
				werr(LOGDEV,"Can't read item %s icon name",ctx->selection.inventory);
			}
		} else {
			if(!entry_read_string(ITEM_TEMPLATE_TABLE,template,&inventory_icon_name,ITEM_ICON,NULL)) {
				werr(LOGDEV,"Can't read item %s icon name (template: %s)",ctx->selection.inventory,template);
			}
			free(template);
		}

		if( inventory_icon_name ) {
			item = item_list_add(&item_list);

			anim = imageDB_get_anim(ctx, inventory_icon_name);
			free(inventory_icon_name);

			item_set_overlay(item,1);
			item_set_anim(item,sw-anim->w,y,anim);
			item_set_click_left(item,show_inventory,NULL,NULL);
		}
	} else {
		if( max_w == 0) {
			max_w = 32;
		}
		if( max_h == 0) {
			max_h = 32;
		}

		if( inventory_icon == NULL ) {
			inventory_icon = anim_create_color(ctx->render, max_w, max_h, 0x7f7f7f7f);
		}

		item = item_list_add(&item_list);

		item_set_overlay(item,1);
		item_set_anim(item,sw-inventory_icon->w,y,inventory_icon);
		item_set_click_left(item,show_inventory,NULL,NULL);
	}

	deep_free(name_list);
}

/**********************************
Compose text
**********************************/
static void compose_text(context_t * ctx)
{
	const history_entry_t * history;
	history_entry_t * hist;
	Uint32 time = SDL_GetTicks();
	int sw;
	int sh;
	int current_y;
	static TTF_Font * font = NULL;
	item_t * item;
	int w;
	int h;
	int x;
	int y;

	if ( font == NULL ) {
		font = TTF_OpenFont(TEXT_FONT, TEXT_FONT_SIZE);
	}
	if ( font == NULL ) {
		return;
	}

	SDL_GetRendererOutputSize(ctx->render,&sw,&sh);
	current_y = sh - action_bar_height;

	/* Draw edit box */
	item = item_list_add(&item_list);

	item_set_overlay(item,1);
	item_set_string(item,text_buffer);
	item_set_font(item,font);
	item_set_editable(item,1);
	item_set_edit_cb(item,keyboard_text);
	sdl_get_string_size(item->font,item->string,&w,&h);
	x = w;
	if ( w < 100 ) {
		x = 100;
	}
	y = h;
	if ( y < TEXT_FONT_SIZE ) {
		y = TEXT_FONT_SIZE;
	}
	item_set_frame_shape(item,0,current_y-y,x,y);
	current_y-=y;
	if(attribute_height > current_y) {
		return;
	}

	/* Draw text history */
	history = textview_get_history();

	if ( history == NULL ) {
		return;
	}

	hist = (history_entry_t*)history;

	while(hist) {
		if ( time > hist->time + TEXT_TIMEOUT ) {
			return;
		}

		item = item_list_add(&item_list);

		item_set_overlay(item,1);
		item_set_string(item,hist->text);
		item_set_font(item,font);
		sdl_get_string_size(item->font,item->string,&w,&h);
		item_set_frame_shape(item,0,current_y-h,w,h);
		current_y-=h;
		if(attribute_height > current_y) {
			return;
		}

		hist = hist->next;
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

	/* Tile selection */
	x = ctx->selection.map_coord[0];
	y = ctx->selection.map_coord[1];

	if( x != -1 && y != -1) {
		item = item_list_add(&item_list);

		/* get pixel coordiante from tile coordianate */
		x = x * ctx->tile_x;
		y = y * ctx->tile_y;

		/* Center on tile */
		x -= (anim->w-ctx->tile_x)/2;
		y -= (anim->h-ctx->tile_y)/2;

		item_set_anim(item,x,y,anim);
	}

	/* Sprite selection */
	if( ctx->selection.id != NULL) {
		item = item_list_add(&item_list);

		if(!entry_read_int(CHARACTER_TABLE,ctx->selection.id,&x,CHARACTER_KEY_POS_X,NULL)) {
			return;
		}
		if(!entry_read_int(CHARACTER_TABLE,ctx->selection.id,&y,CHARACTER_KEY_POS_Y,NULL)) {
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
		/* Empty text buffer */
		text_buffer[0]=0;
		init = 0;
	}

	change_map = ctx->change_map;

	compose_map(ctx);
	compose_item(ctx);
	compose_sprite(ctx);
	compose_select(ctx);

	/* Overlay */
	compose_attribute(ctx);
	compose_action(ctx);
	compose_equipment(ctx);
	compose_text(ctx);

	/* force virtual coordinate on map change */
	if(change_map) {
		sdl_force_virtual_x(ctx->pos_x * ctx->tile_x + ctx->tile_x/2);
		sdl_force_virtual_y(ctx->pos_y * ctx->tile_y + ctx->tile_y/2);
	}
	/* set virtual coordiante on the same map */
	else {
		sdl_set_virtual_x(ctx->cur_pos_x * ctx->tile_x + ctx->tile_x/2);
		sdl_set_virtual_y(ctx->cur_pos_y * ctx->tile_y + ctx->tile_y/2);
	}

	sdl_free_keycb(NULL);
	sdl_add_keycb(SDL_SCANCODE_UP,key_up,NULL,NULL);
	sdl_add_keycb(SDL_SCANCODE_DOWN,key_down,NULL,NULL);
	sdl_add_keycb(SDL_SCANCODE_LEFT,key_left,NULL,NULL);
	sdl_add_keycb(SDL_SCANCODE_RIGHT,key_right,NULL,NULL);
	sdl_add_keycb(SDL_SCANCODE_I,show_inventory,NULL,NULL);
	sdl_add_keycb(SDL_SCANCODE_ESCAPE,cb_quit,NULL,NULL);

	return item_list;
}
