/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013-2014 carabobz@gmail.com

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

#include <lua.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

typedef struct selection {
	char *		id;	/* a character id */
	int		map_coord[2];	/* a tile map */
	char *		map;
	char *		inventory;	/* name of the selected item in inventory */
	char *		equipment;	/* name of the selected slot in equipment */
} selection_t;

typedef struct context {
	char *		user_name;
	int		connected; /* User logged with the correct password, or NPC activated */
	int		in_game;
	TCPsocket	socket;
	TCPsocket	socket_data;
	SDL_mutex*	send_mutex; /* Asynchronous network send */
	char *		hostname;
	int		async_send_num;/* Number of asynchronous data sending thread */
	SDL_mutex*	async_send_mutex;/* mutex for previous data */

	SDL_Renderer * render;
	SDL_Window * window;

	char *		character_name;
	char *		map;	/* map name */
	int		map_w;	/* map width , -1 is an invalid size*/
	int		map_h;	/* map height , -1 is an invalid size*/
	int		tile_x; /* size of a tile for the current map */
	int		tile_y; /* size of a tile for the current map */
	int		pos_x;	/* player position (in tile) */
	int		pos_y;	/* player position (in tile) */
	int		cur_pos_x;	/* player position (in tile) */
	int		cur_pos_y;	/* player position (in tile) */
	int		old_pos_x;	/* player position (in tile) */
	int		old_pos_y;	/* player position (in tile) */
	Uint32		pos_tick;	/* tick for player animation */
	char *		type;	/* character's type */
	selection_t 	selection; /* Selected tile or sprite */
	char *		id; /* unique ID of a character (its filename) */
	char *		prev_map; /* the map from where this context comes */
	int 		change_map; /* Has this context map changed ? */
	lua_State*	luaVM; /* LUA state */
	SDL_cond*	cond;	/* async condition for npc */
	SDL_mutex*	cond_mutex;/* mutex for async condition for npc */
	int		orientation; /* Bit field for sprite orientation (north east, south...)*/
	int		direction; /* Bit field for sprite direction (north, south...)*/

	struct context*	previous;
	struct context*	next;
} context_t;


void context_init(context_t * context);
context_t * context_new(void);
void context_free(context_t * context);
int context_set_hostname(context_t * context, const char * name);
int context_set_username(context_t * context, const char * name);
void context_set_in_game(context_t * context, int in_game);
int context_get_in_game(context_t * context);
void context_set_connected(context_t * context, int connected);
int context_get_connected(context_t * context);
void context_set_socket(context_t * context, TCPsocket socket);
TCPsocket context_get_socket(context_t * context);
void context_set_socket_data(context_t * context, TCPsocket socket);
TCPsocket context_get_socket_data(context_t * context);
int context_set_character_name(context_t * context, const char * name);
int context_set_map(context_t * context, const char * name);
int context_set_map_w(context_t * context, int width);
int context_set_map_h(context_t * context, int height);
int context_set_type(context_t * context, const char * name);
void context_set_pos_x(context_t * context, unsigned int pos);
void context_set_pos_y(context_t * context, unsigned int pos);
void context_set_tile_x(context_t * context, unsigned int pos);
void context_set_tile_y(context_t * context, unsigned int pos);
void context_new_VM(context_t * context);
int context_set_id(context_t * context, const char * name);
int context_update_from_file(context_t * context);
int context_update_from_network_frame(context_t * context, char * frame);
void context_update_from_context(context_t * remote_context);
void context_spread(context_t * context);
void context_add_or_update_from_network_frame(context_t * context, char * data);
void context_lock_list();
void context_unlock_list();
context_t * context_get_first();
context_t * context_get_player();
int context_write_to_file(context_t * context);
void context_broadcast_file(const char * table, const char * file, int same_map_only);
void context_request_other_context(context_t * context);
context_t * context_find(const char * id);
void context_broadcast_text(const char * map, const char * text);
int context_distance(context_t * ctx1, context_t * ctx2);
void context_reset_all_position();

#endif
