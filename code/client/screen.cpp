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

#include <limits.h>
#include <pthread.h>
#include "../sdl_item/sdl.h"
#include "screen.h"
#include "scr_select.h"
#include "scr_play.h"
#include "option_client.h"
#include "lua_client.h"

static bool screen_running = true;
static item_t * item_list = nullptr;
static int current_screen=SCREEN_SELECT;
static bool compose = 0;
static int virtual_x[SCREEN_LAST];
static int virtual_y[SCREEN_LAST];
static double virtual_z[SCREEN_LAST];

#define ITEM_FONT "Ubuntu-C.ttf"
#define ITEM_FONT_SIZE 15

static item_t * frame_rate = nullptr;
static constexpr int const& FPS_DISPLAY_PERIOD = 1000;

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
Called at start of each frame
******************************************************/
static void frame_start(context_t * context)
{
	switch(current_screen) {
	case SCREEN_SELECT:
		scr_select_frame_start(context);
		break;
	case SCREEN_PLAY:
		scr_play_frame_start(context);
		break;
	}
}

/******************************************************
create a list of item for the currently selected screen
******************************************************/
static void compose_scr(context_t * context)
{
	static TTF_Font * font = nullptr;
	option_t * option = option_get();

	SDL_SetRenderDrawColor(context->render, 0, 0, 0, 255);

	switch(current_screen) {
	case SCREEN_SELECT:
		item_list = scr_select_compose(context);
		break;
	case SCREEN_PLAY:
		item_list = scr_play_compose(context);
		break;
	}

	if( option->show_fps ) {
		font = font_get(context,ITEM_FONT, ITEM_FONT_SIZE);
		frame_rate = item_list_add(&item_list);
		item_set_font(frame_rate,font);
		item_set_anim_shape(frame_rate,50,50,20,20);
		item_set_overlay(frame_rate,1);
	}
}

/************************************************
************************************************/
static void display_fps()
{
	static Uint32 timer = 0;
	Uint32 new_timer;
	static char fps[64];
	double sample;
	option_t * option;
	static int num_frame = 0;

	if( frame_rate ) {
		option = option_get();
		if( option->show_fps ) {
			num_frame++;
			new_timer = SDL_GetTicks();
			if( timer + FPS_DISPLAY_PERIOD < new_timer ) {
				sample = (double)num_frame / ((double)new_timer - (double)timer ) * (double)FPS_DISPLAY_PERIOD;
				num_frame = 0;
				timer = new_timer;
				sprintf(fps,"%f",sample);
			}
			item_set_string(frame_rate,fps);
		}
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

	while( screen_running == true) {

		frame_start(ctx);

		if(compose) {
			compose = 0;
			compose_scr(ctx);
		}

		display_fps();

		while (SDL_PollEvent(&event)) {
			compose |= sdl_screen_manager(ctx->window, ctx->render, &event);
			sdl_mouse_manager(ctx->render,&event,item_list);
			sdl_keyboard_manager(&event);
		}

		sdl_mouse_position_manager(ctx->render,item_list);

		SDL_RenderClear(ctx->render);

//		sdl_blit_item_list(ctx->render,item_list);
		item_t * item;

		item = item_list;
		while(item != nullptr)  {
			if( item->user_ptr != nullptr ) {
				context_t * ctx_drawn = (context_t *)item->user_ptr;
				char * draw_script;
				if( entry_read_string(CHARACTER_TABLE,ctx_drawn->id,&draw_script,CHARACTER_KEY_DRAW_SCRIPT,nullptr) != -1)
				{
					item->move_start_tick = 0; // no smooth move
					item->rect.x = item->to_px;
					item->rect.y = item->to_py;

					lua_pushlightuserdata(get_luaVM(),item);
					lua_setglobal (get_luaVM(), "current_item");

					lua_pushlightuserdata(get_luaVM(),ctx_drawn);
					lua_setglobal (get_luaVM(), "current_context");

					if ( lua_execute_script(get_luaVM(), draw_script, nullptr) == -1 ){
						char * l_pTablePath;
						l_pTablePath = strconcat(SCRIPT_TABLE,"/",draw_script,NULL);
						file_lock(l_pTablePath);
						file_update(ctx, l_pTablePath);
						file_unlock(l_pTablePath);
						free(l_pTablePath);
					}
					lua_pop(get_luaVM(),1);
					free(draw_script);
				}
			}

			sdl_blit_item(ctx->render,item);
			item = item->next;
		}

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
	screen_running = false;
}

