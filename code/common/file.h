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

#ifndef FILE_H
#define FILE_H

#define DEFAULT_IMAGE_FILE	"default.jpg"

/* users data */
#define USERS_TABLE		"user"
#define USERS_CHARACTER_LIST    "character_list"

/* password data */
#define PASSWD_TABLE		"passwd"
#define PASSWD_KEY_PASSWORD	"password"

/* attributes */
#define ATTRIBUTE_GROUP		"attribute"
#define ATTRIBUTE_CURRENT	"current"
#define ATTRIBUTE_PREVIOUS	"previous"
#define ATTRIBUTE_MIN		"min"
#define ATTRIBUTE_MAX		"max"
#define ATTRIBUTE_ON_MIN	"on_min"
#define ATTRIBUTE_ON_MAX	"on_max"
#define ATTRIBUTE_ON_UP		"on_up"
#define ATTRIBUTE_ON_DOWN	"on_down"

/* tile data */
#define TILE_TABLE 		"tile"
#define TILE_KEY_IMAGE 		"image"
#define TILE_KEY_TEXT 		"text"
#define TILE_KEY_TYPE 		"type"

/* character data */
#define CHARACTER_TABLE		"character"
#define CHARACTER_KEY_MARQUEE 	"marquee"
#define CHARACTER_KEY_SPRITE	"sprite"
#define CHARACTER_KEY_TYPE	"type"
#define CHARACTER_KEY_MAP	"map"
#define	CHARACTER_KEY_POS_X	"pos_x"
#define CHARACTER_KEY_POS_Y	"pos_y"
#define CHARACTER_KEY_NPC	"npc"
#define CHARACTER_KEY_NAME	"name"
#define CHARACTER_KEY_ACTION	"action"
#define CHARACTER_KEY_ALLOWED_TILE	"allowed_tile"
#define CHARACTER_KEY_INVENTORY	"inventory"
#define CHARACTER_KEY_AI	"ai"
#define CHARACTER_KEY_AI_PARAMS	"ai_params"
#define CHARACTER_KEY_AGGRO_DIST	"aggro_dist"
#define CHARACTER_KEY_AGGRO_SCRIPT	"aggro_script"

#define CHARACTER_TEMPLATE_TABLE	"character_template"

/* misc data */
#define CURSOR_SPRITE_FILE 	"cursor.png"
#define CURSOR_TILE_FILE 	"cursor.png"

/* image DB */
#define IMAGE_TABLE 		"image"

/* map data */
#define MAP_TABLE		"map"

#define MAP_ENTRY_ITEM_LIST 	"item_list"
#define MAP_ENTRY_EVENT_LIST 	"event_list"
/* Tile set definition file : property of each tile (starting with it's associated picture) */
#define MAP_DESCRIPTION 	"description"
#define MAP_KEY_SIZE_X		"size_x"
#define MAP_KEY_SIZE_Y		"size_y"
#define MAP_KEY_TILE_SIZE_X	"tile_size_x"
#define MAP_KEY_TILE_SIZE_Y	"tile_size_y"
#define MAP_KEY_SET		"set"

/* items on map */
#define MAP_ITEM_POS_X		"pos_x"
#define MAP_ITEM_POS_Y		"pos_y"

/* events on map */
#define MAP_EVENT_POS_X		"pos_x"
#define MAP_EVENT_POS_Y		"pos_y"
#define MAP_EVENT_SCRIPT	"script"
#define MAP_EVENT_PARAM		"param"

/* action data */
#define ACTION_TABLE		"action"
#define ACTION_KEY_LIST		"action_list"
#define ACTION_KEY_SCRIPT	"script"
#define ACTION_KEY_TYPE		"type"
#define ACTION_KEY_TEXT		"text"
#define ACTION_KEY_ICON		"icon"
#define ACTION_KEY_PARAM	"params"
#define ACTION_KEY_OPTION	"options"

/* item data */
#define ITEM_TABLE		"item"
#define ITEM_NAME		"name"
#define ITEM_SPRITE		"sprite"
#define ITEM_ICON		"icon"
#define ITEM_DESC		"description"

#define ITEM_TEMPLATE_TABLE	"item_template"

/* scripts */
#define SCRIPT_TABLE		"script"

/* character equipment */
#define EQUIPMENT_GROUP		"equipment"
#define EQUIPMENT_ICON		"icon"
#define EQUIPMENT_NAME		"name"
#define EQUIPMENT_EQUIPPED	"equipped"

#endif
