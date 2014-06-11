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

#include <pthread.h>
#include "sdl.h"
#include "screen.h"
#include "scr_select.h"
#include "scr_play.h"
#include "scr_inventory.h"

static int screen_end = -1;
static item_t * item_list = NULL;
static int current_screen=SCREEN_SELECT;
static int compose = 0;

static void screen_select_compose(context_t * context)
{
	item_t * list;

	list = scr_select_compose(context);

	item_list = list;
}

static void screen_play_compose(context_t * context)
{
	item_t * list;

	list = scr_play_compose(context);

	item_list = list;
}

static void screen_inventory_compose(context_t * context)
{
	item_t * list;

	list = scr_inventory_compose(context);

	item_list = list;
}

/***********************************************
Called by other thread to request compose update
composing create anim object.
anim object has to be in the same thread that
created the renderer (or maybe the window ?)
***********************************************/
void screen_compose()
{
	compose = 1;
}

/******************************************************
create a list of item for the currently selected screen
******************************************************/
static void compose_scr(context_t * context)
{
	switch(current_screen) {
		case SCREEN_SELECT:
			screen_select_compose(context);
		break;
		case SCREEN_PLAY:
			screen_play_compose(context);
		break;
		case SCREEN_INVENTORY:
			screen_inventory_compose(context);
		break;
	}
}

/************************************************
Render the currently selected item list to screen
************************************************/
void screen_display(context_t * ctx)
{
	SDL_Event event;

	while( screen_end == -1) {

		while (SDL_PollEvent(&event)) {
			if(event.type==SDL_KEYDOWN && event.key.keysym.sym==SDLK_ESCAPE) {
				return;
			}

			sdl_screen_manager(ctx,&event);
			sdl_mouse_manager(ctx,&event,item_list);
			sdl_keyboard_manager(&event);
		}

		if(compose) {
			compose = 0;
			compose_scr(ctx);
		}

		SDL_RenderClear(ctx->render);

		sdl_blit_item_list(ctx,item_list);

		sdl_blit_to_screen(ctx);

		sdl_loop_manager();
	}

	return;
}

void screen_set_screen(int screen)
{
	if(screen != current_screen) {
		current_screen = screen;
		screen_compose();
	}
}
