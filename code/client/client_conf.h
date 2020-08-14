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

#ifndef WOG_CLIENT_CONF
#define WOG_CLIENT_CONF

#include <string>

typedef struct
{
	int show_tile_type;
	int show_fps;
	std::string cursor_over_tile;

	std::string cursor_character_draw_script;
	std::string action_select_character;
	std::string cursor_tile;
	std::string action_select_tile;
	std::string cursor_equipment;
	std::string action_select_equipment;
	std::string cursor_inventory;
	std::string action_select_inventory;

	std::string action_move_up;
	std::string action_move_down;
	std::string action_move_left;
	std::string action_move_right;
	std::string action_move_up_right;
	std::string action_move_up_left;
	std::string action_move_down_right;
	std::string action_move_down_left;
} client_conf_t;

void client_conf_init();
client_conf_t & client_conf_get();
void client_conf_read();

#endif
