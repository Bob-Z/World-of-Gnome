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

#ifndef CONTEXT_H
#define CONTEXT_H

class Context;
struct lua_State;

#ifdef __cplusplus
extern "C"
{
#endif
#include <lua.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#ifdef __cplusplus
}
#endif

#include "NetworkFrame.h"
#include "types.h"

typedef struct selection
{
	char * id;	// a character id
	int map_coord[2];	// a tile map
	char * map;
	char * inventory;	// name of the selected item in inventory
	char * equipment;	// name of the selected slot in equipment
} selection_t;

typedef struct context
{
	char * user_name;
	bool connected; // User logged with the correct password, or NPC activated
	bool in_game;
	TCPsocket socket;
	TCPsocket socket_data;
	SDL_mutex* send_mutex; // Asynchronous network send
	char * hostname;

	SDL_Renderer * render;
	SDL_Window * window;

	int npc;	// 1 = NPC
	char * character_name;
	char * map;	// map name
	int tile_x;	// player position (in tile)
	int tile_y;	// player position (in tile)
	int prev_pos_tile_x;// player previous position (in tile) for sprite direction
	int prev_pos_tile_y;// player previous position (in tile) for sprite direction
	bool pos_changed;

	Uint32 animation_tick;	// Start tick for animation

	char * type;	// character's type
	selection_t selection; // Selected tile or sprite
	char * id; // unique ID of a character (its filename)
	char * prev_map; // the map from where this context comes
	bool change_map; // Has this context map changed ?
	lua_State* luaVM;	// LUA state
	SDL_cond* cond;	// async condition for npc
	SDL_mutex* cond_mutex;	// mutex for async condition for npc */
	int orientation;// Bit field for sprite orientation (north east, south...)
	int direction;	// Bit field for sprite direction (north, south...)

	Uint32 next_execution_time; // Time when an NPC will execute its AI script

	struct context* previous;
	struct context* next;
} context_t;

context_t * context_get_list_start();
void context_init(context_t * context);
context_t * context_new(void);
void context_free_data(context_t * context);
void context_free(context_t * context);
ret_code_t context_set_hostname(context_t * context, const char * name);
ret_code_t context_set_username(context_t * context, const char * name);
void context_set_in_game(context_t * context, int in_game);
int context_get_in_game(context_t * context);
void context_set_connected(context_t * context, bool connected);
int context_get_connected(context_t * context);
void context_set_socket(context_t * context, TCPsocket socket);
TCPsocket context_get_socket(context_t * context);
void context_set_socket_data(context_t * context, TCPsocket socket);
TCPsocket context_get_socket_data(context_t * context);
ret_code_t context_set_character_name(context_t * context, const char * name);
ret_code_t context_set_map(context_t * context, const char * name);
int context_set_map_w(context_t * context, int width);
int context_set_map_h(context_t * context, int height);
ret_code_t context_set_type(context_t * context, const char * name);
void context_set_npc(context_t * context, int npc);
void context_set_pos_tx(context_t * context, int pos);
void context_set_pos_ty(context_t * context, int pos);
void context_set_tile_x(context_t * context, unsigned int pos);
void context_set_tile_y(context_t * context, unsigned int pos);
void context_new_VM(context_t * context);
ret_code_t context_set_id(context_t * context, const char * name);
ret_code_t context_set_selected_character(context_t * context,
		const char * selected_character);
ret_code_t context_set_selected_tile(context_t * context,
		const char * selected_map, int selected_map_x, int selected_map_y);
ret_code_t context_set_selected_equipment(context_t * context,
		const char * selected_equipment);
ret_code_t context_set_selected_item(context_t * context,
		const char * selected_item);
ret_code_t context_update_from_file(context_t * context);
void context_spread(context_t * context);
void context_add_or_update_from_network_frame(const Context & context);
void context_lock_list();
void context_unlock_list();
context_t * context_get_first();
context_t * context_get_player();
ret_code_t context_write_to_file(context_t * context);
context_t * context_find(const char * id);
int context_distance(context_t * ctx1, context_t * ctx2);
void context_reset_all_position();
bool context_is_npc(context_t * ctx);

#endif
