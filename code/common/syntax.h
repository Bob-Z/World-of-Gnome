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

#ifndef SYNTAX_H
#define SYNTAX_H

#include <string>

static const std::string USERS_TABLE
{ "user" };
static const std::string USERS_CHARACTER_LIST
{ "character_list" };

static const std::string PASSWD_TABLE
{ "passwd" };
static const std::string PASSWD_KEY_PASSWORD
{ "password" };

static const std::string ATTRIBUTE_GROUP
{ "attribute" };
static const std::string ATTRIBUTE_CURRENT
{ "current" };
static const std::string ATTRIBUTE_PREVIOUS
{ "previous" };
static const std::string ATTRIBUTE_MIN
{ "min" };
static const std::string ATTRIBUTE_MAX
{ "max" };
static const std::string ATTRIBUTE_ON_MIN
{ "on_min" };
static const std::string ATTRIBUTE_ON_MAX
{ "on_max" };
static const std::string ATTRIBUTE_ON_UP
{ "on_up" };
static const std::string ATTRIBUTE_ON_DOWN
{ "on_down" };

// character data
static const std::string CHARACTER_TABLE
{ "character" };
static const std::string CHARACTER_KEY_MARQUEE
{ "marquee" };
static const std::string CHARACTER_KEY_SPRITE
{ "sprite" };
static const std::string CHARACTER_KEY_PORTRAIT
{ "portrait" };
static const std::string CHARACTER_KEY_ZOOM
{ "zoom" };
// sprite align : 0 = center 1 = lower
static const std::string CHARACTER_KEY_ALIGN
{ "align" };
static const std::string CHARACTER_KEY_OFFSET_Y
{ "offset_y" };
static const std::string CHARACTER_KEY_TYPE
{ "type" };
static const std::string CHARACTER_KEY_MAP
{ "map" };
static const std::string CHARACTER_LAYER
{ "layer" };
static const std::string CHARACTER_KEY_TILE_X
{ "tile_x" };
static const std::string CHARACTER_KEY_TILE_Y
{ "tile_y" };
static const std::string CHARACTER_KEY_NPC
{ "npc" };
static const std::string CHARACTER_KEY_NAME
{ "name" };
static const std::string CHARACTER_KEY_ACTION
{ "action" };
static const std::string CHARACTER_KEY_ALLOWED_TILE
{ "allowed_tile" };
static const std::string CHARACTER_KEY_ALLOWED_TILE_SCRIPT
{ "allowed_tile_script" };
static const std::string CHARACTER_KEY_INVENTORY
{ "inventory" };
static const std::string CHARACTER_KEY_AI
{ "ai" };
static const std::string CHARACTER_KEY_AI_PARAMS
{ "ai_params" };
static const std::string CHARACTER_KEY_AGGRO_DIST
{ "aggro_dist" };
static const std::string CHARACTER_KEY_AGGRO_SCRIPT
{ "aggro_script" }; // Called with param: target id, 1 if in aggro, 0 if out of aggro
static const std::string CHARACTER_KEY_DIR_N_ROT
{ "rot_n" };
static const std::string CHARACTER_KEY_DIR_NE_ROT
{ "rot_ne" };
static const std::string CHARACTER_KEY_DIR_E_ROT
{ "rot_e" };
static const std::string CHARACTER_KEY_DIR_SE_ROT
{ "rot_se" };
static const std::string CHARACTER_KEY_DIR_S_ROT
{ "rot_s" };
static const std::string CHARACTER_KEY_DIR_SW_ROT
{ "rot_sw" };
static const std::string CHARACTER_KEY_DIR_W_ROT
{ "rot_w" };
static const std::string CHARACTER_KEY_DIR_NW_ROT
{ "rot_nw" };
static const std::string CHARACTER_KEY_DIR_N_FLIP
{ "flip_n" };
static const std::string CHARACTER_KEY_DIR_S_FLIP
{ "flip_s" };
static const std::string CHARACTER_KEY_DIR_W_FLIP
{ "flip_w" };
static const std::string CHARACTER_KEY_DIR_E_FLIP
{ "flip_e" };
static const std::string CHARACTER_KEY_FORCE_FLIP
{ "force_flip" };
static const std::string CHARACTER_KEY_DIR_N_SPRITE
{ "sprite_n" };
static const std::string CHARACTER_KEY_DIR_S_SPRITE
{ "sprite_s" };
static const std::string CHARACTER_KEY_DIR_W_SPRITE
{ "sprite_w" };
static const std::string CHARACTER_KEY_DIR_E_SPRITE
{ "sprite_e" };
static const std::string CHARACTER_KEY_MOV_N_SPRITE
{ "sprite_move_n" };
static const std::string CHARACTER_KEY_MOV_S_SPRITE
{ "sprite_move_s" };
static const std::string CHARACTER_KEY_MOV_W_SPRITE
{ "sprite_move_w" };
static const std::string CHARACTER_KEY_MOV_E_SPRITE
{ "sprite_move_e" };
static const std::string CHARACTER_KEY_PLATFORM
{ "platform" };
static const std::string CHARACTER_KEY_DRAW_SCRIPT
{ "draw_script" };

static const std::string CHARACTER_TEMPLATE_TABLE
{ "character_template" };

// image DB
static const std::string IMAGE_TABLE
{ "image" };

// map data
static const std::string MAP_TABLE
{ "map" };
static const std::string MAP_DESCRIPTION
{ "description" };
static const std::string MAP_KEY_BG_RED
{ "bg_red" };
static const std::string MAP_KEY_BG_BLUE
{ "bg_blue" };
static const std::string MAP_KEY_BG_GREEN
{ "bg_green" };
static const std::string MAP_KEY_WARP_X
{ "warp_x" };
static const std::string MAP_KEY_WARP_Y
{ "warp_y" };
// layer data*/
static const std::string MAP_KEY_LAYER
{ "layer" };
static const std::string MAP_KEY_WIDTH
{ "width" };
static const std::string MAP_KEY_HEIGHT
{ "height" };
static const std::string MAP_KEY_TILE_WIDTH
{ "tile_width" };
static const std::string MAP_KEY_TILE_HEIGHT
{ "tile_height" };
static const std::string MAP_KEY_SPRITE_ZOOM
{ "sprite_zoom" };
static const std::string MAP_KEY_COL_WIDTH
{ "col_width" };
static const std::string MAP_KEY_COL_HEIGHT
{ "col_height" };
static const std::string MAP_KEY_ROW_WIDTH
{ "row_width" };
static const std::string MAP_KEY_ROW_HEIGHT
{ "row_height" };
static const std::string MAP_KEY_SET
{ "set" };
static const std::string MAP_KEY_SCENERY
{ "scenery" };
static const std::string MAP_KEY_SCENERY_X
{ "x" };
static const std::string MAP_KEY_SCENERY_Y
{ "y" };
static const std::string MAP_KEY_SCENERY_IMAGE
{ "image" };
static const std::string MAP_KEY_TYPE
{ "type" };
static const std::string MAP_ENTRY_ITEM_LIST
{ "g_itemList" };
static const std::string MAP_ENTRY_EVENT_LIST
{ "event_list" };
static const std::string MAP_OFFSCREEN
{ "offscreen" };
static const std::string MAP_SFX
{ "sfx" };
static const std::string MAP_SFX_VOLUME
{ "sfx_volume" };

// items on map
static const std::string MAP_ITEM_TILE_X
{ "tile_x" };
static const std::string MAP_ITEM_TILE_Y
{ "tile_y" };

// events on map
static const std::string MAP_EVENT_TILE_X
{ "tile_x" };
static const std::string MAP_EVENT_TILE_Y
{ "tile_y" };
static const std::string MAP_EVENT_SCRIPT
{ "script" };
static const std::string MAP_EVENT_PARAM
{ "param" };

// action data
static const std::string ACTION_TABLE
{ "action" };
static const std::string ACTION_KEY_SCRIPT
{ "script" };
static const std::string ACTION_KEY_ICON
{ "icon" };
static const std::string ACTION_KEY_ICON_OVER
{ "icon_over" };
static const std::string ACTION_KEY_ICON_CLICK
{ "icon_click" };
static const std::string ACTION_KEY_ICON_LAYOUT
{ "icon_layout" };
static const std::string ACTION_KEY_PARAM
{ "param" };
static const std::string ACTION_KEY_COOLDOWN
{ "cooldown" };

// item data
static const std::string ITEM_TABLE
{ "item" };
static const std::string ITEM_NAME
{ "name" };
static const std::string ITEM_SPRITE
{ "sprite" };
// sprite align : 0 = center 1 = lower
static const std::string ITEM_ALIGN
{ "align" };
static const std::string ITEM_OFFSET_Y
{ "offset_y" };
static const std::string ITEM_ICON
{ "icon" };
static const std::string ITEM_DESC
{ "description" };
static const std::string ITEM_TEMPLATE
{ "template" };
static const std::string ITEM_QUANTITY
{ "quantity" };

static const std::string ITEM_TEMPLATE_TABLE
{ "item_template" };

// scripts
static const std::string SCRIPT_TABLE = "script";

// character equipment
static const std::string EQUIPMENT_GROUP
{ "equipment" };
static const std::string EQUIPMENT_ICON
{ "icon" };
static const std::string EQUIPMENT_NAME
{ "name" };
static const std::string EQUIPMENT_EQUIPPED
{ "equipped" };

// client configuration
static const std::string CLIENT_CONF_FILE
{ "client.json" };
static const std::string CLIENT_KEY_VERSION
{ "version" };
static const std::string CLIENT_KEY_CAMERA_SCRIPT
{ "camera_script" };
static const std::string CLIENT_KEY_CURSOR_OVER_TILE
{ "cursor_over_tile" };
static const std::string CLIENT_KEY_CURSOR_CHARACTER_DRAW_SCRIPT
{ "cursor_character_draw_script" };
static const std::string CLIENT_KEY_CURSOR_TILE
{ "cursor_tile" };
static const std::string CLIENT_KEY_CURSOR_EQUIPMENT
{ "cursor_equipment" };
static const std::string CLIENT_KEY_CURSOR_INVENTORY
{ "cursor_inventory" };
static const std::string CLIENT_KEY_ACTION_MOVE_UP
{ "action_move_up" };
static const std::string CLIENT_KEY_ACTION_MOVE_DOWN
{ "action_move_down" };
static const std::string CLIENT_KEY_ACTION_MOVE_LEFT
{ "action_move_left" };
static const std::string CLIENT_KEY_ACTION_MOVE_RIGHT
{ "action_move_right" };
static const std::string CLIENT_KEY_ACTION_MOVE_UP_RIGHT
{ "action_move_up_right" };
static const std::string CLIENT_KEY_ACTION_MOVE_UP_LEFT
{ "action_move_up_left" };
static const std::string CLIENT_KEY_ACTION_MOVE_DOWN_RIGHT
{ "action_move_down_right" };
static const std::string CLIENT_KEY_ACTION_MOVE_DOWN_LEFT
{ "action_move_down_left" };
static const std::string CLIENT_KEY_ACTION_SELECT_CHARACTER
{ "action_select_character" };
static const std::string CLIENT_KEY_ACTION_SELECT_TILE
{ "action_select_tile" };
static const std::string CLIENT_KEY_ACTION_SELECT_EQUIPMENT
{ "action_select_equipment" };
static const std::string CLIENT_KEY_ACTION_SELECT_INVENTORY
{ "action_select_inventory" };
static const std::string CLIENT_KEY_SELECT_CHARACTER_SFX
{ "select_character_sfx" };
static const std::string CLIENT_KEY_SELECT_CHARACTER_SFX_VOLUME
{ "select_character_sfx_volume" };
static const std::string CLIENT_KEY_SELECT_CHARACTER_ADD_ICON
{ "select_character_add_icon" };
static const std::string CLIENT_KEY_CREATE_CHARACTER_SFX
{ "create_character_sfx" };
static const std::string CLIENT_KEY_CREATE_CHARACTER_SFX_VOLUME
{ "create_character_sfx_volume" };

const std::string SFX_TABLE
{ "sfx" };

#endif
