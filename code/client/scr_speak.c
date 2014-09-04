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
#include <limits.h>
#include "network_client.h"

#define FONT "/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-C.ttf"
#define FONT_SIZE 15

static item_t * item_list = NULL;
static char * name = NULL;
//static anim_t * icon = NULL;
static char * text = NULL;
static speak_entry_t * speak = NULL;
static int speak_num = 0;

/****************************
****************************/
static void clean_up()
{
	int i;

	if( name ) {
		free(name);
		name = NULL;
	}
	if( text ) {
		free(text);
		text = NULL;
	}
	
	if( speak ) {
		for(i=0;i<speak_num;i++) {
			free(speak[i].icon);
			free(speak[i].text);
			free(speak[i].keyword);
		}
		free(speak);
		speak = NULL;
	}

	speak_num = 0;
}

/****************************
****************************/
static void cb_quit(void * arg)
{
	clean_up();
	screen_set_screen(SCREEN_PLAY);
}

/****************************
****************************/
void cb_speak(void * arg)
{
	context_t * player = context_get_list_first();
	char * keyword = (char *)arg;
	char * speak_script = NULL;

	if(!entry_read_string(CHARACTER_TABLE, player->id,&speak_script,CHARACTER_KEY_SPEAK,NULL)) {
		return;
	}

	network_send_action(player, speak_script,keyword);
}

/**********************************
Compose screen
**********************************/
static void compose_screen(context_t * ctx)
{
	item_t * item;
	int i = 0;
	int y = 0;
	static TTF_Font * font = NULL;
	int w;
	int h;

	if ( font == NULL ) {
		font = TTF_OpenFont(FONT, FONT_SIZE);
	}
	
	item = item_list_add(&item_list);
	item_set_string(item,name);
	item_set_font(item,font);
	sdl_get_string_size(item->font,item->string,&w,&h);
	item_set_frame_shape(item,0,y,w,h);
	y += h;

	item = item_list_add(&item_list);
	item_set_string(item,text);
	item_set_font(item,font);
	sdl_get_string_size(item->font,item->string,&w,&h);
	item_set_frame_shape(item,0,y,w,h);
	y += h;
	
	for ( i = 0; i < speak_num; i++) {
		item = item_list_add(&item_list);
		item_set_string(item,speak[i].icon);
		item_set_font(item,font);
		sdl_get_string_size(item->font,item->string,&w,&h);
		item_set_frame_shape(item,0,y,w,h);
		item_set_click_left(item,cb_speak,(void*)speak[i].keyword,NULL);
		y += h;
		
		item = item_list_add(&item_list);
		item_set_string(item,speak[i].text);
		item_set_font(item,font);
		sdl_get_string_size(item->font,item->string,&w,&h);
		item_set_frame_shape(item,0,y,w,h);
		item_set_click_left(item,cb_speak,(void*)speak[i].keyword,NULL);
		y += h;

		item = item_list_add(&item_list);
		item_set_string(item,speak[i].keyword);
		item_set_font(item,font);
		sdl_get_string_size(item->font,item->string,&w,&h);
		item_set_frame_shape(item,0,y,w,h);
		item_set_click_left(item,cb_speak,(void*)speak[i].keyword,NULL);
		y += h;
	}
}

/**********************************
Parse network frame and generate a speak screen
**********************************/
void scr_speak_parse(int total_size, char * frame)
{
	char *saveptr1 = NULL;
	char * s_icon = NULL;
	char * s_text = NULL;
	char * s_keyword = NULL;
	int size = 0;
	
	clean_up();
	
	name = strdup(strtok_r(frame,NETWORK_DELIMITER,&saveptr1));
	size += strlen(name);
	size += strlen(NETWORK_DELIMITER);
	text = strdup(strtok_r(NULL,NETWORK_DELIMITER,&saveptr1));
	size += strlen(text);
	size += strlen(NETWORK_DELIMITER);
	while( size < total_size ) {
		s_icon = strtok_r(NULL,NETWORK_DELIMITER,&saveptr1);
		if(s_icon == NULL) {
			werr(LOGDEBUG,"Parsing error",NULL);
			return;
		}
		size += strlen(s_icon);
		size += strlen(NETWORK_DELIMITER);
		
		s_text = strtok_r(NULL,NETWORK_DELIMITER,&saveptr1);
		if(s_text == NULL) {
			werr(LOGDEBUG,"Parsing error",NULL);
			return;
		}
		size += strlen(s_text);
		size += strlen(NETWORK_DELIMITER);
		
		s_keyword = strtok_r(NULL,NETWORK_DELIMITER,&saveptr1);
		if(s_keyword == NULL) {
			werr(LOGDEBUG,"Parsing error",NULL);
			return;
		}
		size += strlen(s_keyword);
		size += strlen(NETWORK_DELIMITER);

		speak_num++;
		speak = realloc(speak,speak_num*sizeof(speak_entry_t));
		speak[speak_num-1].icon = strdup(s_icon);
		speak[speak_num-1].text = strdup(s_text);
		speak[speak_num-1].keyword = strdup(s_keyword);
	}
	
	screen_set_screen(SCREEN_SPEAK);
	screen_compose();
}

/**********************************
Compose the inventory screen
**********************************/
item_t * scr_speak_compose(context_t * ctx)
{
	int sw = 0;
	int sh = 0;

	if(item_list) {
		item_list_free(item_list);
		item_list = NULL;
	}

	SDL_GetRendererOutputSize(ctx->render,&sw,&sh);

	sdl_force_virtual_x(sw/2);
	sdl_force_virtual_y(sh/2);

	compose_screen(ctx);

	sdl_free_keycb(NULL);
	sdl_add_keycb(SDL_SCANCODE_ESCAPE,cb_quit,NULL,NULL);
	sdl_add_keycb(SDL_SCANCODE_SPACE,cb_quit,NULL,NULL);

	return item_list;
}
