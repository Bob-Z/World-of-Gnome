/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2020 carabobz@gmail.com

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

#include "Camera.h"
#include "entry.h"
#include "file.h"
#include "file_client.h"
#include "font.h"
#include "log.h"
#include "lua_script.h"
#include "lua_client.h"
#include "option_client.h"
#include "scr_create.h"
#include "scr_play.h"
#include "scr_select.h"
#include "screen.h"
#include "sdl.h"
#include "syntax.h"
#include <limits.h>
#include <pthread.h>

static bool screen_running = true;
static item_t * item_list = nullptr;
static bool g_Compose = true;

#define ITEM_FONT "Ubuntu-C.ttf"
#define ITEM_FONT_SIZE 15

static item_t * frame_rate_item = nullptr;
static constexpr int FPS_DISPLAY_PERIOD = 1000;
static Camera g_Camera;

/***********************************************
 Called by other thread to request compose update.
 Composing creates anim object.
 Anim object has to be in the thread that
 created the renderer (or maybe the window ?)
 ***********************************************/
void screen_compose()
{
	g_Compose = true;
}

/******************************************************
 Called at start of each frame
 ******************************************************/
static void frame_start(Context * context)
{
	switch (g_Camera.getScreen())
	{
	case Screen::SELECT:
		scr_select_frame_start(context);
		break;
	case Screen::CREATE:
		scr_create_frame_start(context);
		break;
	case Screen::PLAY:
		scr_play_frame_start(context);
		break;
	default:
		break;
	}
}

/******************************************************
 create a list of item for the currently selected screen
 ******************************************************/
static void compose_scr(Context * context)
{
	static TTF_Font * font = nullptr;

	sdl_set_background_color(0, 0, 0, 255);

	switch (g_Camera.getScreen())
	{
	case Screen::SELECT:
		item_list = scr_select_compose(context);
		break;
	case Screen::CREATE:
		item_list = scr_create_compose(context);
		break;
	case Screen::PLAY:
		item_list = scr_play_compose(context);
		break;
	default:
		break;
	}

	if (option_get().show_fps)
	{
		font = font_get(context, ITEM_FONT, ITEM_FONT_SIZE);
		frame_rate_item = item_list_add(&item_list);
		item_set_font(frame_rate_item, font);
		item_set_anim_shape(frame_rate_item, 100, 50, 20, 20);
		item_set_overlay(frame_rate_item, 1);
	}
}

/******************************************************
 Init screen for first display
 ******************************************************/
static void init_scr()
{
	switch (g_Camera.getScreen())
	{
	case Screen::SELECT:
		scr_select_init();
		break;
	case Screen::CREATE:
		scr_create_init();
		break;
	case Screen::PLAY:
		scr_play_init();
		break;
	default:
		break;
	}
}

/************************************************
 ************************************************/
static void display_fps()
{
	static Uint32 timer = 0;
	Uint32 new_timer;
	static char shown_fps[64];
	double sample;
	static int num_frame = 0;

	if (frame_rate_item != nullptr)
	{
		if (option_get().show_fps)
		{
			num_frame++;
			new_timer = SDL_GetTicks();
			if (timer + FPS_DISPLAY_PERIOD < new_timer)
			{
				sample = (double) num_frame / ((double) new_timer - (double) timer) * (double) FPS_DISPLAY_PERIOD;
				num_frame = 0;
				timer = new_timer;
				sprintf(shown_fps, "%f", sample);
			}
			item_set_string(frame_rate_item, shown_fps);
		}
	}
}

/************************************************
 ************************************************/
void calculate_camera_position(Context * p_pCtx)
{
	char * l_pCameraScript = nullptr;
	entry_read_string(nullptr, CLIENT_CONF_FILE, &l_pCameraScript, CLIENT_KEY_CAMERA_SCRIPT, nullptr);
	if (l_pCameraScript == nullptr || l_pCameraScript[0] == '\0')
	{
		werr(LOGDESIGNER, "No camera script defined. Camera won't move");
	}
	else
	{
		lua_pushlightuserdata(getLuaVm(), (void *) &g_Camera);
		lua_setglobal(getLuaVm(), "current_camera");

		if (lua_execute_script(getLuaVm(), l_pCameraScript, nullptr) == -1)
		{
			file_request_from_network(p_pCtx, SCRIPT_TABLE, l_pCameraScript);
		}
	}
	if (l_pCameraScript != nullptr)
	{
		free(l_pCameraScript);
	}
}

/************************************************
 ************************************************/
static void execute_draw_script(Context * p_pCtx, const char * p_pScriptName, Context * p_pCtxToDraw, item_t * p_pItem)
{
	item_t l_TempItem;
	memcpy(&l_TempItem, p_pItem, sizeof(item_t));

	lua_pushlightuserdata(getLuaVm(), &l_TempItem);
	lua_setglobal(getLuaVm(), "current_item");

	lua_pushlightuserdata(getLuaVm(), p_pCtxToDraw);
	lua_setglobal(getLuaVm(), "current_context");

	if (lua_execute_script(getLuaVm(), p_pScriptName, nullptr) == -1)
	{
		file_request_from_network(p_pCtx, SCRIPT_TABLE, p_pScriptName);
	}

	sdl_blit_item(&l_TempItem);
}

/************************************************
 Render the currently selected item list to screen
 ************************************************/
void screen_display(Context * ctx)
{
	SDL_Event event;

	while (screen_running == true)
	{
		frame_start(ctx);

		if (g_Compose == true)
		{
			g_Compose = false;
			compose_scr(ctx);
		}

		display_fps();

		while (SDL_PollEvent(&event))
		{
			g_Compose |= sdl_screen_manager(&event);
			g_Compose |= sdl_mouse_manager(&event, item_list);
			g_Compose |= sdl_keyboard_manager(&event);
		}

		sdl_mouse_position_manager(item_list);

		sdl_clear();

//		sdl_blit_item_list(item_list);
		item_t * item;

		item = item_list;
		while (item != nullptr)
		{
			if (item->user_ptr != nullptr)
			{
				Context * ctx_drawn = (Context *) item->user_ptr;

				char * draw_script = nullptr;

				// Drawing script not directly attached to context (i.e. for cursor)
				if (item->user1_ptr != nullptr)
				{
					draw_script = strdup((const char *) item->user1_ptr);
				}
				else
				{
					entry_read_string(CHARACTER_TABLE, ctx_drawn->getId().c_str(), &draw_script, CHARACTER_KEY_DRAW_SCRIPT, nullptr);
				}

				if (draw_script != nullptr)
				{
					execute_draw_script(ctx, draw_script, ctx_drawn, item);

					free(draw_script);

					item = item->next;
					continue;
				}
			}

			sdl_blit_item(item);
			item = item->next;
		}

		calculate_camera_position(ctx);

		sdl_blit_to_screen();

		sdl_loop_manager();
	}

	return;
}
/************************************************
 Select the screen to be rendered
 ************************************************/
void screen_set_screen(Screen p_Screen)
{
	if (p_Screen != g_Camera.getScreen())
	{
		g_Camera.setScreen(p_Screen);
		init_scr();
	}
	screen_compose();
}

/************************************************
 End the rendering
 ************************************************/
void screen_quit()
{
	screen_running = false;
}

/************************************************
 ************************************************/
Screen screen_get_current_screen()
{
	return g_Camera.getScreen();
}

/************************************************
 ************************************************/
Camera * screen_get_camera()
{
	return &g_Camera;
}
