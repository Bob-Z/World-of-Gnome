/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013-2017 carabobz@gmail.com

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
#include "imageDB.h"
#include "file.h"
#include "../sdl_item/anim.h"
#include "../sdl_item/item.h"
#include "../sdl_item/sdl.h"
#include "network_client.h"
#include "screen.h"
#include "Camera.h"

static constexpr int const BORDER = 20;
static constexpr int const FONT_SIZE = 30;
static constexpr const char * const FONT = "Ubuntu-C.ttf";

typedef struct {
	char * id;
	char * name;
	char * type;
	anim_t * anim;
	item_t * item;
	int width;
} character_t;

static character_t * character_list = nullptr;
static int character_num = 0;
static item_t * item_list = nullptr;
static long current_character = -1;
static char * sfx_filename = nullptr;

/****************************
Keyboard callback
****************************/
static void cb_quit(void * arg)
{
	screen_quit();
}

/**********************************
**********************************/
static void cb_show_item(void * arg)
{
	item_t * item = (item_t*)arg;

	if( item == nullptr ) {
		return;
	}

	Camera * l_Camera = screen_get_camera();
	l_Camera->setX(item->rect.x + item->rect.w/2);
	l_Camera->setY(item->rect.y + item->rect.h/2);
}

/**********************************
**********************************/
static void cb_select(void * arg)
{
	context_t * ctx = (context_t*)arg;
	character_t  *character;

	if (current_character == -1 ) {
		return;
	}

	character = &(character_list[current_character]);

	context_set_id(ctx,character->id);
	context_set_character_name(ctx, character->name);
	context_set_in_game(ctx,true);

	file_clean(ctx);

	sdl_free_mousecb();

	if( sfx_filename != nullptr ) {
		sfx_stop(ctx,sfx_filename);
	}

	screen_set_screen(Screen::PLAY);
}

/**********************************
**********************************/
static void cb_over(void * arg,int x,int y)
{
	current_character = (long)arg;
}

/**********************************
**********************************/
static void cb_next_character(void * arg)
{
	if ( current_character == -1 ) {
		current_character = 0;
	}

	current_character++;
	if( current_character >= character_num ) {
		current_character = character_num - 1;
	}

	cb_show_item(character_list[current_character].item);
}

/**********************************
**********************************/
static void cb_previous_character(void * arg)
{
	if ( current_character == -1 ) {
		current_character = 0;
	}

	current_character--;
	if( current_character <= 0 ) {
		current_character = 0;
	}

	cb_show_item(character_list[current_character].item);
}

/**********************************
**********************************/
static void cb_wheel_up(Uint32 y, Uint32 unused)
{
	cb_previous_character(nullptr);
}

/**********************************
**********************************/
static void cb_wheel_down(Uint32 y, Uint32 unused)
{
	cb_next_character(nullptr);
}

/**********************************
**********************************/
static void cb_icon_add_clicked(void * arg)
{
	context_t * ctx = (context_t*)arg;

	if( sfx_filename != nullptr ) {
		sfx_stop(ctx,sfx_filename);
	}

	screen_set_screen(Screen::CREATE);
}

/**********************************
**********************************/
void scr_select_frame_start(context_t * context)
{
}

/**********************************
**********************************/
void scr_select_init()
{
	current_character = -1;
}

/**********************************
Compose the character select screen
**********************************/
item_t * scr_select_compose(context_t * context)
{
	long i = 0;
	int x = 0;
	char * marquee_name;
	static int max_h = 0;
	item_t * item;
	item_t * item_image;
	int w;
	int h;
	static TTF_Font * font_name = nullptr;
	static TTF_Font * font_type = nullptr;

	if(character_num==0) {
		return nullptr;
	}

	if( sfx_filename == nullptr ) {
		entry_read_string(nullptr,CLIENT_CONF_FILE,&sfx_filename,CLIENT_KEY_SELECT_CHARACTER_SFX,nullptr);
	}

	if( sfx_filename ) {
		sfx_play(context,sfx_filename,NO_RESTART);

		int sfx_volume = 100; // 100%
		entry_read_int(nullptr,CLIENT_CONF_FILE,&sfx_volume,CLIENT_KEY_SELECT_CHARACTER_SFX_VOLUME,nullptr);
		sfx_set_volume(sfx_volume);
	}

	if(item_list) {
		item_list_free(item_list);
		item_list = nullptr;
	}

	font_name = font_get(context,FONT, FONT_SIZE);
	font_type = font_get(context,FONT, FONT_SIZE);

	sdl_free_mousecb();
	sdl_add_mousecb(MOUSE_WHEEL_UP,cb_wheel_up);
	sdl_add_mousecb(MOUSE_WHEEL_DOWN,cb_wheel_down);

	char * icon_add_image_name = nullptr;
	entry_read_string(nullptr,CLIENT_CONF_FILE,&icon_add_image_name,CLIENT_KEY_SELECT_CHARACTER_ADD_ICON,nullptr);
	if( icon_add_image_name != nullptr ) {
		int sw;
		int sh;

		SDL_GetRendererOutputSize(context->render,&sw,&sh);

		anim_t *anim = imageDB_get_anim(context, icon_add_image_name);
		int x;
		int y;
		x = sw/2-(anim->w/2);
		y = sh-anim->h;

		item = item_list_add(&item_list);
		item_set_overlay(item,1);
		item_set_pos(item,x,y);
		item_set_anim(item,anim,0);

		item_set_click_left(item,cb_icon_add_clicked,(void*)context,nullptr);
		item_set_click_right(item,cb_icon_add_clicked,(void*)context,nullptr);
	}

	SDL_LockMutex(character_select_mutex);

	// Load all anim compute max height and width of anim + string
	for(i=0; i<character_num; i++) {
		// Compute the marquee file name
		if(entry_read_string(CHARACTER_TABLE,character_list[i].id,&marquee_name,CHARACTER_KEY_MARQUEE,nullptr) == RET_NOK ) {
			continue;
		}
		character_list[i].anim  = imageDB_get_anim(context,marquee_name);
		free(marquee_name);

		if(character_list[i].anim->h > max_h) {
			max_h = character_list[i].anim->h;
		}

		if( font_name ) {
			sdl_get_string_size(font_name,character_list[i].name,&w,&h);
			character_list[i].width = w;
		}
		if( font_type ) {
			sdl_get_string_size(font_type,character_list[i].type,&w,&h);
			if( w > character_list[i].width ) {
				character_list[i].width = w;
			}
		}
		if(character_list[i].anim->w > character_list[i].width) {
			character_list[i].width = character_list[i].anim->w;
		}
	}

	// Create item list
	for(i=0; i<character_num; i++) {
		if( character_list[i].anim == nullptr ) {
			continue;
		}

		// Character picture
		item = item_list_add(&item_list);
		item_image = item;
		character_list[i].item = item;

		item_set_pos(item,x+character_list[i].width/2-character_list[i].anim->w/2,
					 max_h/2-character_list[i].anim->h/2);
		item_set_anim(item,character_list[i].anim,0);
		item_set_click_left(item,cb_show_item,(void *)item,nullptr);
		item_set_click_right(item,cb_select,(void *)context,nullptr);
		item_set_double_click_left(item,cb_select,(void *)context,nullptr);
		item_set_over(item,cb_over,(void *)i,nullptr);

		x += character_list[i].width + BORDER;
		// character name
		if( font_name ) {
			item = item_list_add(&item_list);
			item_set_string(item,character_list[i].name);
			item_set_font(item,font_name);
			// display string just above the picture
			sdl_get_string_size(item->font,item->string,&w,&h);
			item_set_anim_shape(item,item_image->rect.x + item_image->rect.w/2 - w/2, item_image->rect.y-h,w,h);
		} else {
			werr(LOGDEV,"Can't open TTF font %s",FONT);
		}

		// character type
		if( font_type ) {
			item = item_list_add(&item_list);
			item_set_string(item,character_list[i].type);
			item_set_font(item,font_type);
			// display string just below the picture
			sdl_get_string_size(item->font,item->string,&w,&h);
			item_set_anim_shape(item,item_image->rect.x + item_image->rect.w/2 - w/2, item_image->rect.y+item_image->rect.h,w,h);
		} else {
			werr(LOGDEV,"Can't open TTF font %s",FONT);
		}
	}

	if( current_character == -1 ) {
		cb_show_item(character_list[0].item);
	}

	SDL_UnlockMutex(character_select_mutex);

	sdl_free_keycb();
	sdl_add_keycb(SDL_SCANCODE_ESCAPE,cb_quit,nullptr,nullptr);
	sdl_add_keycb(SDL_SCANCODE_RIGHT,cb_next_character,nullptr,nullptr);
	sdl_add_keycb(SDL_SCANCODE_LEFT,cb_previous_character,nullptr,nullptr);
	sdl_add_keycb(SDL_SCANCODE_RETURN,cb_select,nullptr,(void *)context);

	return item_list;
}

/*************************
Add a character to the list
the data is a list a 3 strings, the first string is the id of the character (its file name) the second one is the type of the character, the third is the name of the character.
the list ends with an empty string
*************************/
void scr_select_add_user_character(context_t * p_pCtx, char * p_pFrame)
{
	char * l_pId = _strsep(&p_pFrame,NETWORK_DELIMITER);
	char * l_pType = _strsep(&p_pFrame,NETWORK_DELIMITER);
	char * l_pName = _strsep(&p_pFrame,NETWORK_DELIMITER);

	SDL_LockMutex(character_select_mutex);

	character_num++;

	character_list = (character_t*)realloc(character_list,sizeof(character_t)*character_num);

	character_t * new_character = &(character_list[character_num-1]);
	new_character->id = strdup(l_pId);
	new_character->type = strdup(l_pType);
	new_character->name = strdup(l_pName);
	new_character->anim = nullptr;
	new_character->item = nullptr;
	new_character->width = 0;

	wlog(LOGDEV,"Character %s / %s /%s added",new_character->id,new_character->type,new_character->name);

	SDL_UnlockMutex(character_select_mutex);
}

