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

#include <limits.h>
#include <pthread.h>
#include "../sdl_item/sdl.h"
#include "screen.h"
#include "scr_select.h"
#include "scr_play.h"

static int screen_end = -1;
static item_t * item_list = NULL;
static int current_screen=SCREEN_SELECT;
static int compose = 0;
static int virtual_x[SCREEN_LAST];
static int virtual_y[SCREEN_LAST];
static double virtual_z[SCREEN_LAST];

/***********************************************
Called by other thread to request compose update.
Composing creates anim object.
Anim object has to be in the thread that
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
	SDL_SetRenderDrawColor(context->render, 0, 0, 0, 255);

	switch(current_screen) {
	case SCREEN_SELECT:
		item_list = scr_select_compose(context);
		break;
	case SCREEN_PLAY:
		item_list = scr_play_compose(context);
		break;
	}
}

/************************************************
Render the currently selected item list to screen
************************************************/
void screen_display(context_t * ctx)
{
	SDL_Event event;
	int i;

	for(i=0; i<SCREEN_LAST; i++) {
		virtual_x[i] = INT_MAX;
		virtual_y[i] = INT_MAX;
		virtual_z[i] = -1.0;
	}

	while( screen_end == -1) {

		if(compose) {
			compose = 0;
			compose_scr(ctx);
		}

		while (SDL_PollEvent(&event)) {
			compose |= sdl_screen_manager(ctx->window, ctx->render, &event);
			sdl_mouse_manager(ctx->render,&event,item_list);
			sdl_keyboard_manager(&event);
		}

		sdl_mouse_position_manager(ctx->render,item_list);

		SDL_RenderClear(ctx->render);

		sdl_blit_item_list(ctx->render,item_list);

		sdl_blit_to_screen(ctx->render);

		sdl_loop_manager();
	}

	return;
}
/************************************************
Select the screen to be rendered
************************************************/
void screen_set_screen(int screen)
{
	if(screen != current_screen) {
		/* Save current virtual coordinates */
		virtual_x[current_screen] = sdl_get_virtual_x();
		virtual_y[current_screen] = sdl_get_virtual_y();
		virtual_z[current_screen] = sdl_get_virtual_z();

		/* Restore previous virtual coordinate */
		if( virtual_x[screen] != INT_MAX ) {
			sdl_force_virtual_x(virtual_x[screen]);
			sdl_force_virtual_y(virtual_y[screen]);
			sdl_force_virtual_z(virtual_z[screen]);
		}

		current_screen = screen;
		context_reset_all_position();
		screen_compose();
	}
}

/************************************************
End the rendering
************************************************/
void screen_quit()
{
	screen_end = 1;
}
