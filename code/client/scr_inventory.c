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

#define ITEM_FONT "/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-C.ttf"
#define ITEM_FONT_SIZE 15

static item_t * item_list = NULL;
static char ** inventory_list = NULL;

/****************************
****************************/
static void cb_quit(void * arg)
{
	screen_set_screen(SCREEN_PLAY);
}

/****************************
****************************/
void cb_select(void * arg)
{
	context_t * player = context_get_list_first();
	char * item_id = (char *)arg;

	if( player->selection.inventory ) {
		free( player->selection.inventory );
	}
	context_get_list_first()->selection.inventory = strdup(item_id);
	network_send_context(context_get_list_first());
}

/**********************************
Compose inventory
**********************************/
static void compose_inventory(context_t * ctx)
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

	if ( font == NULL ) {
		font = TTF_OpenFont(ITEM_FONT, ITEM_FONT_SIZE);
	}

	deep_free(inventory_list);

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
	       }
	       else {
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

	       quantity = item_get_quantity(inventory_list[i]);
	       sprintf(buf,"%d",quantity);

		item = item_list_add(&item_list);

		item_set_anim(item,x,0,anim);
		item_set_string(item,buf);
		item_set_font(item,font);

		x += anim->w;
		item_set_click_left(item,cb_select,(void*)strdup(inventory_list[i]),free);

		free(description);
		free(label);
		i++;
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
	int i;

	if(ctx->selection.inventory == NULL) {
		return;
	}
	if(ctx->selection.inventory[0] == 0) {
		return;
	}

	if(item_list == NULL) {
		return;
	}

	anim = imageDB_get_anim(ctx,CURSOR_SPRITE_FILE);

	deep_free(inventory_list);

	/* read data from file */
	if(!entry_read_list(CHARACTER_TABLE,ctx->id,&inventory_list, CHARACTER_KEY_INVENTORY,NULL)) {
		return;
	}

	i = 0;
	x = 0;
	item = item_list;
	while( inventory_list[i] && strcmp(inventory_list[i],ctx->selection.inventory) ) {
		x += item->anim->w;
		item=item->next;
		i++;
	}

	if(inventory_list[i]) {
		item = item_list_add(&item_list);
		item_set_anim(item,x,0,anim);
	}
}

/**********************************
Compose the inventory screen
**********************************/
item_t * scr_inventory_compose(context_t * ctx)
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

	compose_inventory(ctx);
	compose_select(ctx);

	sdl_free_keycb(NULL);
	sdl_add_keycb(SDL_SCANCODE_I,cb_quit,NULL,NULL);
	sdl_add_keycb(SDL_SCANCODE_ESCAPE,cb_quit,NULL,NULL);
	sdl_add_keycb(SDL_SCANCODE_SPACE,cb_quit,NULL,NULL);

	return item_list;
}
