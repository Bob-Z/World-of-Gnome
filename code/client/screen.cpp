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
#include "client_conf.h"
#include "entry.h"
#include "global.h"
#include "file_client.h"
#include "file.h"
#include "font.h"
#include "log.h"
#include "lua_client.h"
#include "lua_script.h"
#include "scr_create.h"
#include "scr_play.h"
#include "scr_select.h"
#include "screen.h"
#include "sdl.h"
#include "syntax.h"
#include <limits.h>
#include <memory>

static bool screenRunning = true;
static ItemArray itemArray;
static bool compose = true;

#define ITEM_FONT "Ubuntu-C.ttf"
#define ITEM_FONT_SIZE 15

static std::unique_ptr<SdlItem> frameRateItem;
static constexpr int FPS_DISPLAY_PERIOD = 1000;
static Camera camera;

std::string cameraScriptName;

/******************************************************************************
 Called by other thread to request compose update.
 Composing creates anim object.
 Anim object has to be in the thread that
 created the renderer (or maybe the window ?)
 *****************************************************************************/
void screen_compose()
{
	compose = true;
}

/******************************************************************************
 Drop all items. Must be called before freeing contexts: items may hold
 a user pointer or a callback capturing a Context.
 *****************************************************************************/
void screen_clear_items()
{
	itemArray.clear();
}

/******************************************************************************
 Called at start of each frame
 *****************************************************************************/
static void frame_start(Context * context)
{
	switch (camera.getScreen())
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

/******************************************************************************
 create a list of item for the currently selected screen
 return true if composing succeed
 *****************************************************************************/
static bool compose_scr(Context * context)
{
	screen_clear_items();

	sdl_set_background_color(0, 0, 0, 255);

	switch (camera.getScreen())
	{
	case Screen::SELECT:
		scr_select_compose(context, itemArray);
		break;
	case Screen::CREATE:
		scr_create_compose(context, itemArray);
		break;
	case Screen::PLAY:
		scr_play_compose(context, itemArray);
		break;
	default:
		break;
	}

	bool ret = false;
	if (itemArray.size() != 0)
	{
		ret = true;
	}

	return ret;
}

/******************************************************************************
 Init screen for first display
 *****************************************************************************/
static void init_scr()
{
	sdl_init_screen();

	switch (camera.getScreen())
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

/*****************************************************************************/
static void display_fps(Context * ctx)
{
	if (client_conf_get().show_fps == true)
	{
		static TTF_Font * font = nullptr;
		if (font == nullptr)
		{
			font = font_get(ctx, ITEM_FONT, ITEM_FONT_SIZE);
			return;
		}

		static Uint32 timer = 0;
		static int frameQty = 0;

		frameQty++;
		Uint32 new_timer = SDL_GetTicks();
		if (timer + FPS_DISPLAY_PERIOD < new_timer)
		{
			double sample = (double) frameQty / ((double) new_timer - (double) timer) * (double) FPS_DISPLAY_PERIOD;
			frameQty = 0;
			timer = new_timer;

			frameRateItem = std::make_unique<SdlItem>();
			frameRateItem->setFont(font);
			frameRateItem->setPos(100, 50);
			frameRateItem->setShape(20, 20);
			frameRateItem->setOverlay(true);
			frameRateItem->setText(std::to_string(sample));
		}

		if (frameRateItem != nullptr)
		{
			sdl_blit_item(*frameRateItem);
		}
	}
}

/*****************************************************************************/
static void calculate_camera_position(Context * ctx)
{
	if (cameraScriptName.empty() == true)
	{
		cameraScriptName = getDataManager().getNoExcept<std::string>("", CLIENT_CONF_FILE,
		{ CLIENT_KEY_CAMERA_SCRIPT }, "");

		if (cameraScriptName.empty() == true)
		{
			ERR_DESIGN("No camera script defined. Camera won't move");
			return;
		}
	}

	lua_pushlightuserdata(getLuaVm(), (void *) &camera);
	lua_setglobal(getLuaVm(), "current_camera");

	if (lua_execute_script(getLuaVm(), getLuaVmLock(), cameraScriptName.c_str(), nullptr) == -1)
	{
		file_request_from_network(*(ctx->getConnection()), SCRIPT_TABLE, cameraScriptName);
	}
}

/*****************************************************************************/
static void execute_draw_script(Context * ctx, const std::string & scriptName, Context * ctxToDraw, const SdlItem & item)
{
	SdlItem tempItem = item;

	lua_pushlightuserdata(getLuaVm(), &tempItem);
	lua_setglobal(getLuaVm(), "current_item");

	lua_pushlightuserdata(getLuaVm(), ctxToDraw);
	lua_setglobal(getLuaVm(), "current_context");

	if (lua_execute_script(getLuaVm(), getLuaVmLock(), scriptName.c_str(), nullptr) == -1)
	{
		file_request_from_network(*(ctx->getConnection()), SCRIPT_TABLE, scriptName);
	}

	sdl_blit_item(tempItem);
}

/******************************************************************************
 Render the currently selected item list to screen
 *****************************************************************************/
void screen_display(Context * ctx)
{
	while (screenRunning == true)
	{
		frame_start(ctx);

		if (compose == true)
		{
			if (compose_scr(ctx) == true)
			{
				compose = false;
			}
		}

		SDL_Event event;

		while (SDL_PollEvent(&event) == 1)
		{
			compose |= sdl_screen_manager(&event);
			compose |= sdl_mouse_manager(&event, itemArray);
			compose |= sdl_keyboard_manager(&event);
		}

		sdl_mouse_position_manager(itemArray);

		sdl_clear();

		for (auto && item : itemArray)
		{
			if (item->getUserPtr() != nullptr)
			{
				Context * ctx_drawn = (Context *) item->getUserPtr();

				std::string draw_script;

				// Drawing script not directly attached to context (i.e. for cursor)
				if (item->getUserString().empty() == false)
				{
					draw_script = item->getUserString();
				}
				else
				{
					getDataManager().tryGet<std::string>(CHARACTER_TABLE,
							ctx_drawn->getId(), { CHARACTER_KEY_DRAW_SCRIPT },
							draw_script);
				}

				if (draw_script.empty() == false)
				{
					execute_draw_script(ctx, draw_script, ctx_drawn, *item);

					continue;
				}
			}

			sdl_blit_item(*item);
		}

		display_fps(ctx);

		calculate_camera_position(ctx);

		sdl_blit_to_screen();

		sdl_loop_manager();
	}

	return;
}
/*****************************************************************************/
void screen_set_screen(Screen screen)
{
	if (screen != camera.getScreen())
	{
		LOG("Set screen to " + SCREEN_NAME.at(screen));
		camera.setScreen(screen);
		init_scr();
	}
	screen_compose();
}

/*****************************************************************************/
void screen_quit()
{
	screenRunning = false;
}

/*****************************************************************************/
Screen screen_get_current_screen()
{
	return camera.getScreen();
}

/*****************************************************************************/
Camera * screen_get_camera()
{
	return &camera;
}
