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

#include <SDL_stdinc.h>
#include "Selection.h"

class ContextBis;
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

#include "types.h"
#include <SDL_mutex.h>

class Context
{
public:
	Context();
	~Context();
	const std::string& getUserName() const;
	void setUserName(const std::string& userName);
	bool isConnected() const;
	void setConnected(bool connected);

private:
	SDL_mutex* m_mutex;
	std::string m_userName;
	bool m_connected; // User logged with the correct password, or NPC activated

public:
	bool m_in_game;
	TCPsocket m_socket;
	TCPsocket m_socket_data;
	SDL_mutex* m_send_mutex; // Asynchronous network send
	char * m_hostname;

	SDL_Renderer * m_render;
	SDL_Window * m_window;

	int m_npc; // 1 = NPC
	char * m_character_name;
	char * m_map;	// map name
	int m_tile_x;	// player position (in tile)
	int m_tile_y;	// player position (in tile)
	int m_prev_pos_tile_x;	// player previous position (in tile) for sprite direction
	int m_prev_pos_tile_y;	// player previous position (in tile) for sprite direction
	bool m_pos_changed;

	Uint32 m_animation_tick;	// Start tick for animation

	char * m_type;	// character's type
	Selection m_selection; // Selected tile or sprite
	char * m_id; // unique ID of a character (its filename)
	char * m_prev_map; // the map from where this context comes
	bool m_change_map; // Has this context map changed ?
	lua_State* m_lua_VM;	// LUA state
	SDL_cond* m_condition;	// async condition for npc
	SDL_mutex* m_condition_mutex;	// mutex for async condition for npc */
	int m_orientation;	// Bit field for sprite orientation (north east, south...)
	int m_direction;	// Bit field for sprite direction (north, south...)

	Uint32 m_next_execution_time; // Time when an NPC will execute its AI script

	Context * m_previous;
	Context * m_next;

};

Context * context_get_list_start();
void context_init(Context * context);
Context * context_new(void);
void context_free_data(Context * context);
void context_free(Context * context);
ret_code_t context_set_hostname(Context * context, const char * name);
void context_set_in_game(Context * context, bool in_game);
int context_get_in_game(Context * context);
void context_set_socket(Context * context, TCPsocket socket);
TCPsocket context_get_socket(Context * context);
void context_set_socket_data(Context * context, TCPsocket socket);
TCPsocket context_get_socket_data(Context * context);
ret_code_t context_set_character_name(Context * context, const char * name);
ret_code_t context_set_map(Context * context, const char * name);
int context_set_map_w(Context * context, int width);
int context_set_map_h(Context * context, int height);
ret_code_t context_set_type(Context * context, const char * name);
void context_set_npc(Context * context, bool npc);
void context_set_pos_tx(Context * context, int pos);
void context_set_pos_ty(Context * context, int pos);
void context_set_tile_x(Context * context, unsigned int pos);
void context_set_tile_y(Context * context, unsigned int pos);
void context_new_VM(Context * context);
ret_code_t context_set_id(Context * context, const char * name);
ret_code_t context_set_selected_character(Context * context, const char * selected_character);
ret_code_t context_set_selected_tile(Context * context, const char * selected_map, int selected_map_x, int selected_map_y);
ret_code_t context_set_selected_equipment(Context * context, const char * selected_equipment);
ret_code_t context_set_selected_item(Context * context, const char * selected_item);
ret_code_t context_update_from_file(Context * context);
void context_spread(Context * context);
void context_add_or_update_from_network_frame(const ContextBis & context);
void context_lock_list();
void context_unlock_list();
Context * context_get_first();
Context * context_get_player();
ret_code_t context_write_to_file(Context * context);
Context * context_find(const char * id);
int context_distance(Context * ctx1, Context * ctx2);
void context_reset_all_position();
bool context_is_npc(Context * ctx);

#endif
