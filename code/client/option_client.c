/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013-2015 carabobz@gmail.com

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

option_t option;
int already_parsed = 0;

/**************************************
**************************************/
void option_init()
{
	option.show_tile_type = false;
	option.cursor_over_tile = NULL;
	option.cursor_sprite = NULL;
	option.cursor_tile = NULL;
	option.cursor_equipment = NULL;
	option.cursor_inventory = NULL;
}

/**************************************
**************************************/
static void parse_client_conf()
{
	int version;

	if( already_parsed ) {
		return;
	}

	if (!entry_read_int(NULL,CLIENT_CONF_FILE,&version,CLIENT_KEY_VERSION,NULL)) {
		return;
	}

	entry_read_string(NULL,CLIENT_CONF_FILE,&option.cursor_over_tile,CLIENT_KEY_CURSOR_OVER_TILE,NULL);
	entry_read_string(NULL,CLIENT_CONF_FILE,&option.cursor_sprite,CLIENT_KEY_CURSOR_SPRITE,NULL);
	entry_read_string(NULL,CLIENT_CONF_FILE,&option.cursor_tile,CLIENT_KEY_CURSOR_TILE,NULL);
	entry_read_string(NULL,CLIENT_CONF_FILE,&option.cursor_equipment,CLIENT_KEY_CURSOR_EQUIPMENT,NULL);
	entry_read_string(NULL,CLIENT_CONF_FILE,&option.cursor_inventory,CLIENT_KEY_CURSOR_INVENTORY,NULL);
	entry_read_string(NULL,CLIENT_CONF_FILE,&option.action_move_up,CLIENT_KEY_ACTION_MOVE_UP,NULL);
	entry_read_string(NULL,CLIENT_CONF_FILE,&option.action_move_down,CLIENT_KEY_ACTION_MOVE_DOWN,NULL);
	entry_read_string(NULL,CLIENT_CONF_FILE,&option.action_move_left,CLIENT_KEY_ACTION_MOVE_LEFT,NULL);
	entry_read_string(NULL,CLIENT_CONF_FILE,&option.action_move_right,CLIENT_KEY_ACTION_MOVE_RIGHT,NULL);
	entry_read_string(NULL,CLIENT_CONF_FILE,&option.action_move_up_left,CLIENT_KEY_ACTION_MOVE_UP_LEFT,NULL);
	entry_read_string(NULL,CLIENT_CONF_FILE,&option.action_move_up_right,CLIENT_KEY_ACTION_MOVE_UP_RIGHT,NULL);
	entry_read_string(NULL,CLIENT_CONF_FILE,&option.action_move_down_left,CLIENT_KEY_ACTION_MOVE_DOWN_LEFT,NULL);
	entry_read_string(NULL,CLIENT_CONF_FILE,&option.action_move_down_right,CLIENT_KEY_ACTION_MOVE_DOWN_RIGHT,NULL);
	entry_read_string(NULL,CLIENT_CONF_FILE,&option.action_select_character,CLIENT_KEY_ACTION_SELECT_CHARACTER,NULL);
	entry_read_string(NULL,CLIENT_CONF_FILE,&option.action_select_tile,CLIENT_KEY_ACTION_SELECT_TILE,NULL);
	entry_read_string(NULL,CLIENT_CONF_FILE,&option.action_select_equipment,CLIENT_KEY_ACTION_SELECT_EQUIPMENT,NULL);
	entry_read_string(NULL,CLIENT_CONF_FILE,&option.action_select_inventory,CLIENT_KEY_ACTION_SELECT_INVENTORY,NULL);

	already_parsed = 1;
}

/**************************************
**************************************/
option_t * option_get()
{
	parse_client_conf();

	return &option;
}
