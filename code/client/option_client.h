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

#ifndef WOG_OPTION_CLIENT
#define WOG_OPTION_CLIENT

typedef struct
{
	int show_tile_type;
	int show_fps;
	char * cursor_over_tile;

	char * cursor_character_draw_script;
	char * action_select_character;
	char * cursor_tile;
	char * action_select_tile;
	char * cursor_equipment;
	char * action_select_equipment;
	char * cursor_inventory;
	char * action_select_inventory;

	char * action_move_up;
	char * action_move_down;
	char * action_move_left;
	char * action_move_right;
	char * action_move_up_right;
	char * action_move_up_left;
	char * action_move_down_right;
	char * action_move_down_left;
} option_t;

void option_init();
option_t & option_get();
void option_read_client_conf();

#endif
