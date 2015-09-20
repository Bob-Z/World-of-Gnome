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
#include "../sdl_item/anim.h"
#include "../sdl_item/item.h"
#include "../sdl_item/sdl.h"
#include "network_client.h"
#include "imageDB.h"
#include "screen.h"
#include "scr_play.h"
#include "textview.h"
#include "option_client.h"

#define UI_MAIN		0
#define UI_INVENTORY	1
#define UI_POPUP	2

#define FONT "/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-C.ttf"
#define FONT_SIZE 30
#define TEXT_FONT "/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-C.ttf"
#define TEXT_FONT_SIZE 15
#define TEXT_TIMEOUT 5000 /* Text display timeout */
#define ITEM_FONT "/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-C.ttf"
#define ITEM_FONT_SIZE 15
#define SPEAK_FONT "/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-C.ttf"
#define SPEAK_FONT_SIZE 32

#define BACKGROUND_COLOR (0x000000C0)

#define POPUP_TAG_IMAGE		"image"
#define POPUP_TAG_TEXT		"text"
#define POPUP_TAG_ACTION	"action"
#define POPUP_TAG_EOL		"eol"
#define POPUP_TAG_EOP		"eop"
#define POPUP_TAG_END		"popup_end"

static int current_ui = UI_MAIN;
static char * last_action = NULL;
/* main ui */
static char ** attribute_string = NULL;
static int action_bar_height;
static int attribute_height;
static char text_buffer[2048];
/* inventory ui */
static char ** inventory_list = NULL;
/* popup ui */
#define MOUSE_WHEEL_SCROLL (20)
static fifo_t * popup_fifo;
static char * popup_frame = NULL;
static int  popup_active = false;
typedef struct action_param_tag {
	char * action;
	char * param;
} action_param_t;
static int popup_offset = 0;

static option_t * option;

/**********************************
**********************************/
static void draw_background(context_t * ctx, item_t * item_list)
{
	static anim_t * bg_anim = NULL;
	int sw;
	int sh;
	item_t * item;

	if( bg_anim ) {
		si_anim_free(bg_anim);
	}
	SDL_GetRendererOutputSize(ctx->render,&sw,&sh);
	bg_anim = anim_create_color(ctx->render, sw, sh, BACKGROUND_COLOR);
	item = item_list_add(&item_list);
	item_set_anim(item,0,0,bg_anim);
	item_set_overlay(item,1);
}

/**********************************
**********************************/
void ui_play_set(int ui_type)
{
	current_ui = ui_type;
	screen_compose();
}

/**********************************
**********************************/
int ui_play_get()
{
	return current_ui;
}

/**********************************
**********************************/
char * ui_play_get_last_action()
{
	return last_action;
}

/****************************
****************************/
static void cb_main_quit(void * arg)
{
	context_t * current_ctx;
	context_t * next_ctx;

	if( ui_play_get() == UI_MAIN ) {
		context_set_in_game(context_get_player(),false);
		network_request_stop(context_get_player());
		current_ctx = context_get_first();
		while( current_ctx != NULL ) {
			/* Save next before freeing the current context */
			next_ctx = current_ctx->next;
			if( current_ctx != context_get_player() ) {
				context_free(current_ctx);
			}
			current_ctx = next_ctx;
		}

		scr_play_init(true);
		screen_set_screen(SCREEN_SELECT);
	}
}

/****************************
****************************/
static void key_up(void * arg)
{
	context_t * ctx = context_get_player();

	network_send_action(ctx,option->action_move_up,NULL);
}

/**************************************
**************************************/
static void key_down(void * arg)
{
	context_t * ctx = context_get_player();

	network_send_action(ctx,option->action_move_down,NULL);
}

/**************************************
**************************************/
static void key_left(void * arg)
{
	context_t * ctx = context_get_player();

	network_send_action(ctx,option->action_move_left,NULL);
}

/**************************************
**************************************/
static void key_right(void * arg)
{
	context_t * ctx = context_get_player();

	network_send_action(ctx,option->action_move_right,NULL);
}

/**************************************
**************************************/
static void key_up_left(void * arg)
{
	context_t * ctx = context_get_player();

	network_send_action(ctx,option->action_move_up_left,NULL);
}

/**************************************
**************************************/
static void key_up_right(void * arg)
{
	context_t * ctx = context_get_player();

	network_send_action(ctx,option->action_move_up_right,NULL);
}

/**************************************
**************************************/
static void key_down_left(void * arg)
{
	context_t * ctx = context_get_player();

	network_send_action(ctx,option->action_move_down_left,NULL);
}

/**************************************
**************************************/
static void key_down_right(void * arg)
{
	context_t * ctx = context_get_player();

	network_send_action(ctx,option->action_move_down_right,NULL);
}

/**********************************
Compose attribute
**********************************/
static void compose_attribute(context_t * ctx, item_t * item_list)
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
void ui_play_cb_action(void * arg)
{
	char * action = (char *)arg;

	network_send_action(context_get_player(),action,NULL);

	if( last_action ) {
		free(last_action);
		last_action = NULL;
	}

	if( arg ) {
		last_action = strdup(arg);
	} else {
		last_action = NULL;
	}
}

/**********************************
Compose action icon
**********************************/
static void compose_action(context_t * ctx,item_t * item_list)
{
	char ** action_list = NULL;
	char * text = NULL;
	char * icon = NULL;
	char * icon_over = NULL;
	char * icon_click = NULL;
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

		item = item_list_add(&item_list);
		item_set_overlay(item,1);

		/* load image */
		anim = imageDB_get_anim(ctx, icon);
		item_set_anim(item,x,sh-anim->h,anim);

		x += anim->w;
		item_set_click_left(item,ui_play_cb_action,(void*)strdup(action_list[i]),free);
		if( action_bar_height < anim->h ) {
			action_bar_height = anim->h;
		}

		entry_read_string(ACTION_TABLE,action_list[i],&icon_over,ACTION_KEY_ICON_OVER,NULL);
		if( icon_over ) {
			anim = imageDB_get_anim(ctx, icon_over);
			item_set_anim_over(item,anim);
		}

		entry_read_string(ACTION_TABLE,action_list[i],&icon_click,ACTION_KEY_ICON_CLICK,NULL);
		if( icon_click ) {
			anim = imageDB_get_anim(ctx, icon_click);
			item_set_anim_click(item,anim);
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
	context_t * ctx = context_get_player();

	network_send_action(ctx,option->action_select_equipment,id,NULL);
}

/**************************************
**************************************/
static void show_inventory(void * arg)
{
	ui_play_set(UI_INVENTORY);
}

/**********************************
Compose equipment icon
**********************************/
static void compose_equipment(context_t * ctx, item_t * item_list)
{
	char ** slot_list = NULL;
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
	option_t * option = option_get();

	SDL_GetRendererOutputSize(ctx->render,&sw,&sh);

	entry_get_group_list(CHARACTER_TABLE,ctx->id,&slot_list,EQUIPMENT_GROUP,NULL);

	max_w = 0;
	max_h = 0;
	index=0;
	while( slot_list && slot_list[index] != NULL) {
#if 0
		/* Get the slot name */
		if(!entry_read_string(CHARACTER_TABLE,ctx->id,&item_name,EQUIPMENT_GROUP,slot_list[index],EQUIPMENT_NAME,NULL)) {
			name = strdup(slot_list[index]);
		} else {
			name = item_name;
		}
		free(item_name);
#endif
		h1 = 0;
		/* Get the slot icon */
		if(!entry_read_string(CHARACTER_TABLE,ctx->id,&icon_name,EQUIPMENT_GROUP,slot_list[index],EQUIPMENT_ICON,NULL)) {
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

			item_set_click_left(item,cb_select_slot,strdup(slot_list[index]),NULL);

			if(anim->w > max_w) {
				max_w = anim->w;
			}
			if(anim->h > max_h) {
				max_h = anim->h;
			}
		}

		/* Is there an equipped object ? */
		if(entry_read_string(CHARACTER_TABLE,ctx->id,&equipped_name,EQUIPMENT_GROUP,slot_list[index],EQUIPMENT_EQUIPPED,NULL) && equipped_name[0]!=0 ) {
#if 0
			/* Get the equipped object name */
			if(!entry_read_string(ITEM_TABLE,equipped_name,&equipped_text,ITEM_NAME,NULL)) {
				werr(LOGDEV,"Can't read object %s name in equipment slot %s",equipped_name,slot_list[index]);
			}
			free(equipped_text);
#endif
			/* Get its icon */
			template = item_is_resource(equipped_name);

			if ( template == NULL ) {
				if(!entry_read_string(ITEM_TABLE,equipped_name,&equipped_icon_name,ITEM_ICON,NULL)) {
					werr(LOGDEV,"Can't read object %s icon in equipment slot %s",equipped_name,slot_list[index]);
				}
			} else {
				if(!entry_read_string(ITEM_TEMPLATE_TABLE,template,&equipped_icon_name,ITEM_ICON,NULL)) {
					werr(LOGDEV,"Can't read item %s icon name (template: %s)",equipped_name,template);
				}
				free(template);
			}

			if( equipped_icon_name ) {
				item = item_list_add(&item_list);

				anim2 = imageDB_get_anim(ctx, equipped_icon_name);
				free(equipped_icon_name);

				item_set_overlay(item,1);
				item_set_anim(item,x-anim->w,y,anim2);
				item_set_click_left(item,cb_select_slot,strdup(slot_list[index]),NULL);
				if(h1 < anim->h) {
					h1 = anim->h;
				}
			}
			free(equipped_name);
		}

		/* Draw selection cursor */
		if( ctx->selection.equipment[0] != 0) {
			if( option && option->cursor_equipment ) {
				if( !strcmp(ctx->selection.equipment,slot_list[index]) ) {
					anim3 = imageDB_get_anim(ctx,option->cursor_equipment);

					item = item_list_add(&item_list);

					/* Center on icon */
					item_set_overlay(item,1);
					item_set_anim(item,x - (anim3->w-anim->w)/2, y - (anim3->h-anim->w)/2, anim3);
				}
			}
		}

		if(h1 > anim->h) {
			y += h1;
		} else {
			y += anim->h;
		}

		index++;
	}
	deep_free(slot_list);

	/* Draw selected item */
	if( ctx->selection.inventory[0] != 0) {
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
}

/**************************************
**************************************/
static void keyboard_text(void * arg)
{
	char * text = (char*)arg;

	network_send_action(context_get_player(),WOG_CHAT,text,NULL);
	text_buffer[0]=0;
	screen_compose();
}

/**********************************
Compose text
**********************************/
static void compose_text(context_t * ctx, item_t * item_list)
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
	item_set_string_bg(item,BACKGROUND_COLOR);
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
		item_set_string_bg(item,BACKGROUND_COLOR);
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

/****************************
****************************/
static void cb_print_coord(void * arg)
{
	char buf[SMALL_BUF];
	char *type;
	int map_w;
	int player_layer = 0;
	char layer_name[SMALL_BUF];
	context_t * ctx = context_get_player();

	entry_read_int(MAP_TABLE,ctx->map,&player_layer,MAP_CHARACTER_LAYER,NULL);
	sprintf(layer_name,"%s%d",MAP_KEY_LAYER,player_layer);
	entry_read_int(MAP_TABLE,ctx->map,&map_w,layer_name,MAP_KEY_WIDTH,NULL);

	entry_read_list_index(MAP_TABLE,ctx->map,&type,scr_play_get_current_x()+scr_play_get_current_y()*map_w,layer_name,MAP_KEY_TYPE,NULL);
	sprintf(buf,"x=%d y=%d type=%s",scr_play_get_current_x(),scr_play_get_current_y(),type);
	free(type);
	textview_add_line(buf);

	screen_compose();
}

/**********************************
**********************************/
static void main_compose(context_t * ctx, item_t * item_list)
{
	compose_attribute(ctx,item_list);
	compose_action(ctx,item_list);
	compose_equipment(ctx,item_list);
	compose_text(ctx,item_list);

	sdl_add_keycb(SDL_SCANCODE_I,show_inventory,NULL,NULL);
	sdl_add_keycb(SDL_SCANCODE_UP,key_up,NULL,NULL);
	sdl_add_keycb(SDL_SCANCODE_KP_8,key_up,NULL,NULL);
	sdl_add_keycb(SDL_SCANCODE_DOWN,key_down,NULL,NULL);
	sdl_add_keycb(SDL_SCANCODE_KP_2,key_down,NULL,NULL);
	sdl_add_keycb(SDL_SCANCODE_LEFT,key_left,NULL,NULL);
	sdl_add_keycb(SDL_SCANCODE_KP_4,key_left,NULL,NULL);
	sdl_add_keycb(SDL_SCANCODE_RIGHT,key_right,NULL,NULL);
	sdl_add_keycb(SDL_SCANCODE_KP_6,key_right,NULL,NULL);
	sdl_add_keycb(SDL_SCANCODE_KP_7,key_up_left,NULL,NULL);
	sdl_add_keycb(SDL_SCANCODE_KP_9,key_up_right,NULL,NULL);
	sdl_add_keycb(SDL_SCANCODE_KP_1,key_down_left,NULL,NULL);
	sdl_add_keycb(SDL_SCANCODE_KP_3,key_down_right,NULL,NULL);
	sdl_add_keycb(SDL_SCANCODE_ESCAPE,cb_main_quit,NULL,NULL);
	sdl_add_keycb(SDL_SCANCODE_SCROLLLOCK,cb_print_coord,NULL,NULL);
}

/****************************
****************************/
static void cb_inventory_quit(void * arg)
{
	ui_play_set(UI_MAIN);
}

/****************************
****************************/
void cb_inventory_select(void * arg)
{
	char * item_id = (char *)arg;
	context_t * ctx = context_get_player();

	network_send_action(ctx,option->action_select_inventory,item_id,NULL);
}

/**********************************
Compose inventory
**********************************/
static void compose_inventory(context_t * ctx,item_t * item_list)
{
	char * value = NULL;
	char * label;
	char * description;
	anim_t * anim;
	item_t * item;
	int x=0;
	int i = 0;
	static TTF_Font * font = NULL;
	char * template;
	int quantity;
	char buf[1024];
	int w;
	int h;

	if ( font == NULL ) {
		font = TTF_OpenFont(ITEM_FONT, ITEM_FONT_SIZE);
	}

	deep_free(inventory_list);

	draw_background(ctx,item_list);

	/* read data from file */
	if(!entry_read_list(CHARACTER_TABLE,ctx->id,&inventory_list, CHARACTER_KEY_INVENTORY,NULL)) {
		return;
	}

	while( inventory_list[i] != NULL) {
		template = item_is_resource(inventory_list[i]);

		if( template == NULL ) {
			/* Icon is mandatory for now */
			if(!entry_read_string(ITEM_TABLE,inventory_list[i],&value,ITEM_ICON,NULL)) {
				i++;
				continue;
			}
			/* load image */
			anim = imageDB_get_anim(ctx, value);
			free(value);

			if(!entry_read_string(ITEM_TABLE,inventory_list[i],&value,ITEM_NAME,NULL)) {
				label = strdup(inventory_list[i]);
			} else {
				label = value;
			}

			if(!entry_read_string(ITEM_TABLE,inventory_list[i],&value,ITEM_DESC,NULL)) {
				description = strdup("");;
			} else {
				description = value;
			}
		} else {
			/* Icon is mandatory for now */
			if(!entry_read_string(ITEM_TEMPLATE_TABLE,template,&value,ITEM_ICON,NULL)) {
				i++;
				free(template);
				continue;
			}
			/* load image */
			anim = imageDB_get_anim(ctx, value);
			free(value);

			if(!entry_read_string(ITEM_TEMPLATE_TABLE,template,&value,ITEM_NAME,NULL)) {
				label = strdup(inventory_list[i]);
			} else {
				label = value;
			}

			if(!entry_read_string(ITEM_TEMPLATE_TABLE,template,&value,ITEM_DESC,NULL)) {
				description = strdup("");;
			} else {
				description = value;
			}
			free(template);

		}

		quantity = resource_get_quantity(inventory_list[i]);

		if( quantity > 0 ) {
			w = 0;
			item = item_list_add(&item_list);
			item_set_anim(item,x,0,anim);
			if( quantity > 1 ) {
				sprintf(buf,"%d",quantity);
				item_set_string(item,buf);
				item_set_string_bg(item,BACKGROUND_COLOR);
				item_set_font(item,font);
				sdl_get_string_size(item->font,item->string,&w,&h);
			}
			item_set_overlay(item,1);
			if( w > anim->w ) {
				x += w;
			} else {
				x += anim->w;
			}
			item_set_click_left(item,cb_inventory_select,(void*)strdup(inventory_list[i]),free);
		}

		free(description);
		free(label);
		i++;
	}
}

/**********************************
Compose select cursor
**********************************/
static void compose_inventory_select(context_t * ctx,item_t * item_list)
{
	item_t * item;
	int x;
	int i;
	char * icon_name;
	anim_t * anim;
	anim_t * icon_anim;
	char * template;
	option_t * option = option_get();

	if(ctx->selection.inventory[0] == 0) {
		return;
	}

	if(item_list == NULL) {
		return;
	}

	if ( option == NULL || option->cursor_inventory == NULL ) {
		return;
	}

	anim = imageDB_get_anim(ctx,option->cursor_inventory);

	deep_free(inventory_list);

	/* read data from file */
	if(!entry_read_list(CHARACTER_TABLE,ctx->id,&inventory_list, CHARACTER_KEY_INVENTORY,NULL)) {
		return;
	}

	i = 0;
	x = 0;
	while( inventory_list[i] && strcmp(inventory_list[i],ctx->selection.inventory) ) {
		template = item_is_resource(inventory_list[i]);

		if ( template == NULL ) {
			if(!entry_read_string(ITEM_TABLE,inventory_list[i],&icon_name,ITEM_ICON,NULL)) {
				werr(LOGDEV,"Can't read item %s icon name",inventory_list[i]);
			}
		} else {
			if(!entry_read_string(ITEM_TEMPLATE_TABLE,template,&icon_name,ITEM_ICON,NULL)) {
				werr(LOGDEV,"Can't read item %s icon name (template: %s)",inventory_list[i],template);
			}
			free(template);
		}

		icon_anim = imageDB_get_anim(ctx,icon_name);
		free(icon_name);

		x += icon_anim->w;
		i++;
	}

	if(inventory_list[i]) {
		item = item_list_add(&item_list);
		item_set_anim(item,x,0,anim);
		item_set_overlay(item,1);
	}
}

/**********************************
**********************************/
static void inventory_compose(context_t * ctx, item_t * item_list)
{
	compose_inventory(ctx,item_list);
	compose_inventory_select(ctx,item_list);

	sdl_add_keycb(SDL_SCANCODE_I,cb_inventory_quit,NULL,NULL);
	sdl_add_keycb(SDL_SCANCODE_ESCAPE,cb_inventory_quit,NULL,NULL);
	sdl_add_keycb(SDL_SCANCODE_SPACE,cb_inventory_quit,NULL,NULL);
}

/****************************
****************************/
static void cb_popup_quit(void * arg)
{
	if( popup_frame ) {
		free( popup_frame);
		popup_frame = NULL;
	}

	popup_offset = 0;

	popup_frame = fifo_pop(&popup_fifo);

	if( popup_frame == NULL) {
		ui_play_set(UI_MAIN);
	} else {
		popup_active = true;
	}
}

/****************************
****************************/
void cb_popup(void * arg)
{
	context_t * player = context_get_player();
	action_param_t * action_param = (action_param_t *)arg;

	if( !strcmp(action_param->action,POPUP_TAG_END) ) {
		cb_popup_quit(NULL);
		return;
	}

	network_send_action(player, action_param->action,action_param->param,NULL);

	popup_active = false;

	screen_compose();
}

/****************************
****************************/
void cb_free_action_param(void * arg)
{
	action_param_t * action_param = (action_param_t *)arg;

	free(action_param->action);
	free(action_param->param);
	free(action_param);
}

/**********************************
**********************************/
static void cb_wheel_up(Uint32 y, Uint32 unused)
{
	popup_offset -= MOUSE_WHEEL_SCROLL;
	if( popup_offset < 0 ) {
		popup_offset = 0;
	}
	screen_compose();
}

/**********************************
**********************************/
static void cb_wheel_down(Uint32 y, Uint32 unused)
{
	popup_offset += MOUSE_WHEEL_SCROLL;
	screen_compose();
}

/**********************************
Compose screen
**********************************/
static void compose_popup(context_t * ctx,item_t * item_list)
{
	item_t * item;
	int x = 0;
	int y = 0;
	static TTF_Font * font = NULL;
	int w = 0;
	int h = 0;
	int max_h = 0;
	anim_t * anim;
	char * tag;
	action_param_t * action_param = NULL;
	char * data;

	if( popup_frame == NULL ) {
		return;
	}

	draw_background(ctx,item_list);

	if ( font != NULL ) {
		TTF_CloseFont(font);
		font = NULL;
	}
	font = TTF_OpenFont(SPEAK_FONT, SPEAK_FONT_SIZE );

	sdl_free_mousecb();
	sdl_add_mousecb(MOUSE_WHEEL_UP,cb_wheel_up);
	sdl_add_mousecb(MOUSE_WHEEL_DOWN,cb_wheel_down);

	data = strdup(popup_frame);

	while( (tag = _strsep(&data,NETWORK_DELIMITER)) != NULL) {
		if(!strcmp(tag,POPUP_TAG_IMAGE)) {
			/* get image name */
			tag = _strsep(&data,NETWORK_DELIMITER);
			item = item_list_add(&item_list);
			anim = imageDB_get_anim(ctx,tag);
			item_set_anim(item,x,y-popup_offset,anim);
			item_set_overlay(item,1);
			if(action_param) {
				item_set_click_left(item,cb_popup,action_param,cb_free_action_param);
			}
			x += anim->w;
			if( max_h < anim->h ) {
				max_h = anim->h;
			}

			action_param = NULL;
			continue;
		}
		if(!strcmp(tag,POPUP_TAG_TEXT)) {
			/* get text */
			tag = _strsep(&data,NETWORK_DELIMITER);
			item = item_list_add(&item_list);
			item_set_string(item,tag);
			item_set_font(item,font);
			sdl_get_string_size(item->font,item->string,&w,&h);
			item_set_frame_shape(item,x,y-popup_offset,w,h);
			item_set_overlay(item,1);
			if(action_param) {
				item_set_click_left(item,cb_popup,action_param,cb_free_action_param);
			}
			x += w;
			if( max_h < h ) {
				max_h = h;
			}
			action_param = NULL;
			continue;
		}
		if(!strcmp(tag,POPUP_TAG_ACTION)) {
			action_param = malloc(sizeof(action_param_t));
			/* get action */
			tag = _strsep(&data,NETWORK_DELIMITER);
			action_param->action = strdup(tag);
			/* get param */
			tag = _strsep(&data,NETWORK_DELIMITER);
			action_param->param = strdup(tag);
			continue;
		}
		if(!strcmp(tag,POPUP_TAG_EOL)) {
			y += max_h;
			max_h = 0;
			x = 0;
		}
		if(!strcmp(tag,POPUP_TAG_EOP)) {
			y += max_h;
			max_h = 0;
			x = 0;
		}
	}
	free(data);
}

/**********************************
**********************************/
void ui_play_popup_add(char * frame)
{
	if(popup_frame == NULL) {
		popup_frame = strdup(frame);
	} else {
		if(popup_active) {
			fifo_push(&popup_fifo,strdup(frame));
		} else {
			free(popup_frame);
			popup_frame = strdup(frame);
			popup_active = true;
		}
	}
	ui_play_set(UI_POPUP);
}

/**********************************
**********************************/
static void popup_compose(context_t * ctx, item_t * item_list)
{
	compose_popup(ctx,item_list);

	sdl_add_keycb(SDL_SCANCODE_ESCAPE,cb_popup_quit,NULL,NULL);
	sdl_add_keycb(SDL_SCANCODE_SPACE,cb_popup_quit,NULL,NULL);
}
/**********************************
**********************************/
void ui_play_compose(context_t * ctx, item_t * item_list)
{
	switch ( current_ui ) {
	case UI_MAIN:
		main_compose(ctx,item_list);
		break;
	case UI_INVENTORY:
		inventory_compose(ctx,item_list);
		break;
	case UI_POPUP:
		popup_compose(ctx,item_list);
		break;
	default:
		werr(LOGUSER,"Wrong UI type");
		break;
	}
}
/**********************************
Called once
**********************************/
void ui_play_init()
{
	/* Empty text buffer */
	text_buffer[0]=0;

	option = option_get();
}
