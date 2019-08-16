/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2019 carabobz@gmail.com

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

#include "option_client.h"
#include "entry.h"
#include "log.h"
#include "syntax.h"
#include <unistd.h>

option_t option;

/**************************************
 **************************************/
void option_init()
{
	option.show_tile_type = false;
	option.show_fps = false;
	option.cursor_over_tile = nullptr;
	option.cursor_character_draw_script = nullptr;
	option.cursor_tile = nullptr;
	option.cursor_equipment = nullptr;
	option.cursor_inventory = nullptr;
}

/**************************************
 **************************************/
option_t & option_get()
{
	return option;
}

/**************************************
 **************************************/
static void parse_client_conf()
{
	int version;

	while (entry_read_int(nullptr, CLIENT_CONF_FILE, &version,
	CLIENT_KEY_VERSION, nullptr) == RET_NOK)
	{
		wlog(LOGUSER, "Waiting for client configuration");
		usleep(100000);
	}

	entry_read_string(nullptr, CLIENT_CONF_FILE, &option.cursor_over_tile,
	CLIENT_KEY_CURSOR_OVER_TILE, nullptr);
	entry_read_string(nullptr, CLIENT_CONF_FILE, &option.cursor_character_draw_script,
	CLIENT_KEY_CURSOR_CHARACTER_DRAW_SCRIPT, nullptr);
	entry_read_string(nullptr, CLIENT_CONF_FILE, &option.cursor_tile,
	CLIENT_KEY_CURSOR_TILE, nullptr);
	entry_read_string(nullptr, CLIENT_CONF_FILE, &option.cursor_equipment,
	CLIENT_KEY_CURSOR_EQUIPMENT, nullptr);
	entry_read_string(nullptr, CLIENT_CONF_FILE, &option.cursor_inventory,
	CLIENT_KEY_CURSOR_INVENTORY, nullptr);
	entry_read_string(nullptr, CLIENT_CONF_FILE, &option.action_move_up,
	CLIENT_KEY_ACTION_MOVE_UP, nullptr);
	entry_read_string(nullptr, CLIENT_CONF_FILE, &option.action_move_down,
	CLIENT_KEY_ACTION_MOVE_DOWN, nullptr);
	entry_read_string(nullptr, CLIENT_CONF_FILE, &option.action_move_left,
	CLIENT_KEY_ACTION_MOVE_LEFT, nullptr);
	entry_read_string(nullptr, CLIENT_CONF_FILE, &option.action_move_right,
	CLIENT_KEY_ACTION_MOVE_RIGHT, nullptr);
	entry_read_string(nullptr, CLIENT_CONF_FILE, &option.action_move_up_left,
	CLIENT_KEY_ACTION_MOVE_UP_LEFT, nullptr);
	entry_read_string(nullptr, CLIENT_CONF_FILE, &option.action_move_up_right,
	CLIENT_KEY_ACTION_MOVE_UP_RIGHT, nullptr);
	entry_read_string(nullptr, CLIENT_CONF_FILE, &option.action_move_down_left,
	CLIENT_KEY_ACTION_MOVE_DOWN_LEFT, nullptr);
	entry_read_string(nullptr, CLIENT_CONF_FILE, &option.action_move_down_right,
	CLIENT_KEY_ACTION_MOVE_DOWN_RIGHT, nullptr);
	entry_read_string(nullptr, CLIENT_CONF_FILE, &option.action_select_character, CLIENT_KEY_ACTION_SELECT_CHARACTER, nullptr);
	entry_read_string(nullptr, CLIENT_CONF_FILE, &option.action_select_tile,
	CLIENT_KEY_ACTION_SELECT_TILE, nullptr);
	entry_read_string(nullptr, CLIENT_CONF_FILE, &option.action_select_equipment, CLIENT_KEY_ACTION_SELECT_EQUIPMENT, nullptr);
	entry_read_string(nullptr, CLIENT_CONF_FILE, &option.action_select_inventory, CLIENT_KEY_ACTION_SELECT_INVENTORY, nullptr);
}

/**************************************
 **************************************/
void option_read_client_conf()
{
	parse_client_conf();
}
