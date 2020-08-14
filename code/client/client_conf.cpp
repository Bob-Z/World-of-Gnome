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

#include "client_conf.h"
#include "client_server.h"
#include "entry.h"
#include "log.h"
#include "syntax.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <unistd.h>

#include <iostream>

using json = nlohmann::json;

client_conf_t clientConf;

/*****************************************************************************/
void client_conf_init()
{
	clientConf.show_tile_type = false;
	clientConf.show_fps = false;
}

/*****************************************************************************/
client_conf_t & client_conf_get()
{
	return clientConf;
}

/*****************************************************************************/
static void parse_client_conf()
{
	json json;

	do
	{
		try
		{
			std::ifstream stream(base_directory + "/" + CLIENT_CONF_FILE);
			stream >> json;
		} catch (...)
		{
			LOG_USER("Waiting for client configuration");
			usleep(100000);
		}
	} while (json.is_null() == true);

	int version = json.at(CLIENT_KEY_VERSION).get<int>();
	LOG_USER("version = " + std::to_string(version));

	clientConf.cursor_over_tile = json.at(CLIENT_KEY_CURSOR_OVER_TILE).get<std::string>();
	clientConf.cursor_character_draw_script = json.at(CLIENT_KEY_CURSOR_CHARACTER_DRAW_SCRIPT).get<std::string>();
	clientConf.cursor_tile = json.at(CLIENT_KEY_CURSOR_TILE).get<std::string>();
	clientConf.cursor_equipment = json.at(CLIENT_KEY_CURSOR_EQUIPMENT).get<std::string>();
	clientConf.cursor_inventory = json.at(CLIENT_KEY_CURSOR_INVENTORY).get<std::string>();
	clientConf.action_move_up = json.at(CLIENT_KEY_ACTION_MOVE_UP).get<std::string>();
	clientConf.action_move_down = json.at(CLIENT_KEY_ACTION_MOVE_DOWN).get<std::string>();
	clientConf.action_move_left = json.at(CLIENT_KEY_ACTION_MOVE_LEFT).get<std::string>();
	clientConf.action_move_right = json.at(CLIENT_KEY_ACTION_MOVE_RIGHT).get<std::string>();
	clientConf.action_move_up_left = json.at(CLIENT_KEY_ACTION_MOVE_UP_LEFT).get<std::string>();
	clientConf.action_move_up_right = json.at(CLIENT_KEY_ACTION_MOVE_UP_RIGHT).get<std::string>();
	clientConf.action_move_down_left = json.at(CLIENT_KEY_ACTION_MOVE_DOWN_LEFT).get<std::string>();
	clientConf.action_move_down_right = json.at(CLIENT_KEY_ACTION_MOVE_DOWN_RIGHT).get<std::string>();
	clientConf.action_select_character = json.at(CLIENT_KEY_ACTION_SELECT_CHARACTER).get<std::string>();
	clientConf.action_select_tile = json.at(CLIENT_KEY_ACTION_SELECT_TILE).get<std::string>();
	clientConf.action_select_equipment = json.at(CLIENT_KEY_ACTION_SELECT_EQUIPMENT).get<std::string>();
	clientConf.action_select_inventory = json.at(CLIENT_KEY_ACTION_SELECT_INVENTORY).get<std::string>();
}

/*****************************************************************************/
void client_conf_read()
{
	parse_client_conf();
}
