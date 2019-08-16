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

#include "context.h"
#include "ContextBis.h"
#include "entry.h"
#include "log.h"
#include "mutex.h"
#include "syntax.h"
#include <cstring>
#include <stdlib.h>
#include <string>

class ContextBis;

extern "C"
{
#include "lualib.h"
#include "lauxlib.h"
}
#include <unistd.h>
#include <limits.h>
#include "common.h"

context_t * context_list_start = nullptr;

/***********************
 ***********************/
context_t::context_t() :
		user_name(nullptr), connected(false), in_game(false), socket(), socket_data(), send_mutex(nullptr), hostname(nullptr), render(nullptr), window(nullptr), npc(
				true), character_name(nullptr), map(nullptr), tile_x(-1), tile_y(-1), prev_pos_tile_x(-1), prev_pos_tile_y(-1), pos_changed(false), animation_tick(
				0), type(nullptr), selection(), id(nullptr), prev_map(nullptr), change_map(false), luaVM(nullptr), cond(nullptr), cond_mutex(nullptr), orientation(
				0), direction(0), next_execution_time(0), previous(nullptr), next(nullptr)
{
}

/***********************
 ***********************/
context_t * context_get_list_start()
{
	return context_list_start;
}

/***********************
 ***********************/
void context_unlock_list()
{
	SDL_UnlockMutex(context_list_mutex);
}

/***********************
 ***********************/
context_t * context_get_first()
{
	return context_list_start;
}
/*************************************
 context_init
 Initialize a context_t struct
 *************************************/
void context_init(context_t * context)
{
	context->user_name = nullptr;
	context->connected = false;
	context->in_game = false;
	context->socket = 0;
	context->socket_data = 0;
	context->hostname = nullptr;
	context->send_mutex = SDL_CreateMutex();

	context->render = nullptr;
	context->window = nullptr;

	context->npc = 1;
	context->character_name = nullptr;
	context->map = nullptr;

	context->tile_x = 0;
	context->tile_y = 0;
	context->prev_pos_tile_x = 0;
	context->prev_pos_tile_y = 0;
	context->pos_changed = false;
	context->animation_tick = 0;
	context->type = nullptr;

	context->id = nullptr;
	context->prev_map = nullptr;
	context->change_map = false;
	context->luaVM = nullptr;
	context->cond = nullptr;
	context->cond_mutex = nullptr;
	context->orientation = 0;
	context->direction = 0;
	context->next_execution_time = 0;

	context->previous = nullptr;
	context->next = nullptr;
}

/**************************************
 Add a new context to the list
 **************************************/
context_t * context_new(void)
{
	context_t * ctx;

	context_lock_list();
	if (context_list_start == nullptr)
	{
		context_list_start = new context_t;
		context_init(context_list_start);
		context_unlock_list();
		return context_list_start;
	}

	ctx = context_list_start;
	while (ctx->next != nullptr)
	{
		ctx = ctx->next;
	}

	ctx->next = new context_t;
	context_init(ctx->next);
	ctx->next->previous = ctx;
	context_unlock_list();
	return ctx->next;
}
/*************************************
 context_free_data
 Deep free of all context_t data
 *************************************/
void context_free_data(context_t * context)
{
	if (context->user_name)
	{
		free(context->user_name);
	}
	context->user_name = nullptr;
	context->in_game = false;
	context->connected = false;
	if (context->socket != 0)
	{
		SDLNet_TCP_Close(context->socket);
	}
	context->socket = 0;
	if (context->socket_data != 0)
	{
		SDLNet_TCP_Close(context->socket_data);
	}
	context->socket_data = 0;
	SDL_DestroyMutex(context->send_mutex);

	if (context->hostname)
	{
		free(context->hostname);
	}
	context->hostname = nullptr;
	if (context->character_name)
	{
		free(context->character_name);
	}
	context->character_name = nullptr;
	if (context->map)
	{
		free(context->map);
	}
	context->map = nullptr;
	if (context->type)
	{
		free(context->type);
	}
	context->type = nullptr;

	context->prev_map = nullptr;
	if (context->luaVM != nullptr)
	{
		lua_close(context->luaVM);
	}
	if (context->cond != nullptr)
	{
		SDL_DestroyCond(context->cond);
	}
	if (context->cond_mutex != nullptr)
	{
		SDL_DestroyMutex(context->cond_mutex);
	}

	if (context->id)
	{
		free(context->id);
	}
	context->id = nullptr;
}

/*************************************
 context_free
 Deep free of a context_t struct and remove it from context list
 *************************************/
void context_free(context_t * context)
{
	context_t * ctx;

	context_lock_list();

	// First context of the list
	if (context->previous == nullptr)
	{
		context_list_start = context->next;
		if (context->next != nullptr)
		{
			context->next->previous = nullptr;
		}
	}
	else
	{
		context->previous->next = context->next;
		if (context->next != nullptr)
		{
			context->next->previous = context->previous;
		}
	}

	// Remove this context from other context's selection
	ctx = context_list_start;
	while (ctx != nullptr)
	{
		if ((context->id != nullptr) && (context->selection.getId() != ""))
		{
			if (strcmp(context->id, ctx->selection.getId().c_str()) == 0)
			{
				ctx->selection.setId("");
			}
		}
		ctx = ctx->next;
	}

	// Remove this context from the list
	if (context == context_get_first())
	{
		context_list_start = context->next;
	}
	else
	{
		ctx = context_list_start;
		while (ctx != nullptr)
		{
			if (ctx->next == context)
			{
				ctx->next = context->next;
				break;
			}
			ctx = ctx->next;
		}
	}

	context_unlock_list();

	context_free_data(context);

	free(context);
}

/***********************
 ***********************/
void context_lock_list()
{
	SDL_LockMutex(context_list_mutex);
}

/***********************
 ***********************/
context_t * context_get_player()
{
	return context_list_start;
}
/**************************
 Returns RET_NOK if error
 **************************/
ret_code_t context_set_hostname(context_t * context, const char * name)
{
	context_lock_list();

	if (context->hostname)
	{
		free(context->hostname);
	}

	context->hostname = strdup(name);
	if (context->hostname == nullptr)
	{
		context_unlock_list();
		return RET_NOK;
	}

	context_unlock_list();
	return RET_OK;
}

/**************************
 Returns RET_NOK if error
 **************************/
ret_code_t context_set_username(context_t * context, const char * name)
{
	context_lock_list();

	free(context->user_name);
	context->user_name = strdup(name);
	if (context->user_name == nullptr)
	{
		context_unlock_list();
		return RET_NOK;
	}

	context_unlock_list();
	return RET_OK;
}

/**************************************
 **************************************/
void context_set_in_game(context_t * context, int in_game)
{
	context_lock_list();
	context->in_game = in_game;
	context_unlock_list();
}

/**************************************
 **************************************/
int context_get_in_game(context_t * context)
{
	int in_game = 0;

	context_lock_list();
	in_game = context->in_game;
	context_unlock_list();

	return in_game;
}

/**************************************
 **************************************/
void context_set_connected(context_t * context, bool connected)
{
	context_lock_list();
	context->connected = connected;
	context_unlock_list();
}

/**************************************
 **************************************/
int context_get_connected(context_t * context)
{
	int conn = 0;

	context_lock_list();
	conn = context->connected;
	context_unlock_list();

	return conn;
}

/**************************************
 **************************************/
void context_set_socket(context_t * context, TCPsocket socket)
{
	context_lock_list();
	context->socket = socket;
	context_unlock_list();
}

/**************************************
 **************************************/
TCPsocket context_get_socket(context_t * context)
{
	TCPsocket socket = 0;

	context_lock_list();
	socket = context->socket;
	context_unlock_list();

	return socket;
}

/**************************************
 **************************************/
void context_set_socket_data(context_t * context, TCPsocket socket)
{
	context_lock_list();
	context->socket_data = socket;
	context_unlock_list();
}

/**************************************
 **************************************/
TCPsocket context_get_socket_data(context_t * context)
{
	TCPsocket socket = 0;

	context_lock_list();
	socket = context->socket_data;
	context_unlock_list();

	return socket;
}

/**************************************
 Returns RET_NOK if error
 **************************************/
ret_code_t context_set_character_name(context_t * context, const char * name)
{
	ret_code_t ret = RET_OK;

	context_lock_list();
	free(context->character_name);
	context->character_name = strdup(name);
	if (context->character_name == nullptr)
	{
		ret = RET_NOK;
	}
	context_unlock_list();

	return ret;
}

/**************************************
 Returns RET_NOK if error
 **************************************/
static ret_code_t _context_set_map(context_t * context, const char * map)
{
	if (context->prev_map != nullptr)
	{
		if (!strcmp(context->map, map))
		{
			return RET_OK;
		}
		free(context->prev_map);
		context->prev_map = nullptr;
	}

	if (context->map)
	{
		context->prev_map = context->map;
	}

	context->map = strdup(map);
	if (context->map == nullptr)
	{
		return RET_NOK;
	}

	context->change_map = true;

	return RET_OK;
}

/**************************************
 **************************************/
ret_code_t context_set_map(context_t * context, const char * map)
{
	ret_code_t ret;

	context_lock_list();
	ret = _context_set_map(context, map);
	context_unlock_list();

	return ret;
}

/**************************************
 Returns RET_NOK if error
 **************************************/
ret_code_t context_set_type(context_t * context, const char * type)
{
	ret_code_t ret = RET_OK;

	context_lock_list();
	free(context->type);
	context->type = strdup(type);
	if (context->type == nullptr)
	{
		ret = RET_NOK;
	}
	context_unlock_list();

	return ret;
}

/**************************************
 **************************************/
void _context_set_npc(context_t * context, int npc)
{
	context->npc = npc;
}

/**************************************
 **************************************/
void context_set_npc(context_t * context, int npc)
{
	context_lock_list();
	_context_set_npc(context, npc);
	context_unlock_list();
}

/**************************************
 **************************************/
void _context_set_pos_tx(context_t * context, int pos_tx)
{
	context->prev_pos_tile_x = context->tile_x;
	if (context->tile_x != pos_tx)
	{
		context->pos_changed = true;
		context->tile_x = pos_tx;
	}
}

/**************************************
 **************************************/
void context_set_pos_tx(context_t * context, int pos_tx)
{
	context_lock_list();
	_context_set_pos_tx(context, pos_tx);
	context_unlock_list();
}
/**************************************
 **************************************/
void _context_set_pos_ty(context_t * context, int pos_ty)
{
	context->prev_pos_tile_y = context->tile_y;
	if (context->tile_y != pos_ty)
	{
		context->pos_changed = true;
		context->tile_y = pos_ty;
	}
}

/**************************************
 **************************************/
void context_set_pos_ty(context_t * context, int pos_ty)
{
	context_lock_list();
	_context_set_pos_ty(context, pos_ty);
	context_unlock_list();
}

/**************************************
 return RET_NOK on error
 **************************************/
ret_code_t context_set_id(context_t * context, const char * name)
{
	context_lock_list();

	if (context->id)
	{
		free(context->id);
	}
	context->id = strdup(name);
	if (context->id == nullptr)
	{
		context_unlock_list();
		return RET_NOK;
	}

	context_unlock_list();
	return RET_OK;
}

/**************************************
 * return RET_NOK if context has already the same values
 **************************************/
ret_code_t context_set_selected_character(context_t * context, const char * id)
{
	context_lock_list();

	if ((context != nullptr) && (context->selection.getId() != ""))
	{
		if (strcmp(context->selection.getId().c_str(), id) == 0)
		{
			context_unlock_list();
			return RET_NOK;
		}
		context->selection.setId(std::string(id));
	}

	context_unlock_list();
	return RET_OK;
}

/**************************************
 * return RET_NOK if context has already the same values
 **************************************/
ret_code_t context_set_selected_tile(context_t * context, const char * map, int x, int y)
{
	context_lock_list();

	if (!strcmp(context->selection.getMap().c_str(), map))
	{
		if ((x == context->selection.getMapCoordTx()) && (y == context->selection.getMapCoordTy()))
		{
			context_unlock_list();
			return RET_NOK;
		}
	}

	context->selection.setMap(std::string(map));
	context->selection.setMapCoordTx(x);
	context->selection.setMapCoordTy(y);

	context_unlock_list();
	return RET_OK;
}

/**************************************
 * return RET_NOK if context has already the same values
 **************************************/
ret_code_t context_set_selected_equipment(context_t * context, const char * id)
{
	context_lock_list();

	if (strcmp(context->selection.getEquipment().c_str(), id) == 0)
	{
		context_unlock_list();
		return RET_NOK;
	}

	context->selection.setEquipment(id);

	context_unlock_list();
	return RET_OK;
}

/**************************************
 * return RET_NOK if context has already the same values
 **************************************/
ret_code_t context_set_selected_item(context_t * context, const char * id)
{
	context_lock_list();

	if (strcmp(context->selection.getInventory().c_str(), id) == 0)
	{
		context_unlock_list();
		return RET_NOK;
	}

	context->selection.setInventory(id);

	context_unlock_list();
	return RET_OK;
}

/**************************************
 **************************************/
// Client use only one LUA VM, not one LUA VM by context
#ifdef SERVER
void register_lua_functions(context_t * context);
#endif

/**************************************
 **************************************/
void context_new_VM(context_t * context)
{
	context_lock_list();
	context->luaVM = lua_open();
	lua_baselibopen(context->luaVM);
	lua_tablibopen(context->luaVM);
	lua_iolibopen(context->luaVM);
	lua_strlibopen(context->luaVM);
	lua_mathlibopen(context->luaVM);

	register_lua_functions(context);
	context_unlock_list();
}

/*******************************
 Update the memory context by reading the character's data file on disk
 Return RET_NOK if there is an error
 *******************************/
ret_code_t context_update_from_file(context_t * context)
{
	// Don't call context_set_* functions here to avoid inter-blocking

	char * result;
	ret_code_t ret = RET_OK;

	context_lock_list();

	if (context->id == nullptr)
	{
		context_unlock_list();
		return RET_NOK;
	}

	if (entry_read_string(CHARACTER_TABLE, context->id, &result,
	CHARACTER_KEY_NAME, nullptr) == RET_OK)
	{
		free(context->character_name);
		context->character_name = result;
	}
	else
	{
		ret = RET_NOK;
	}

	int npc = 0;
	if (entry_read_int(CHARACTER_TABLE, context->id, &npc, CHARACTER_KEY_NPC, nullptr) == RET_NOK)
	{
		ret = RET_NOK;
	}
	_context_set_npc(context, npc);

	if (entry_read_string(CHARACTER_TABLE, context->id, &result,
	CHARACTER_KEY_TYPE, nullptr) == RET_OK)
	{
		free(context->type);
		context->type = result;
	}
	else
	{
		ret = RET_NOK;
	}

	if (entry_read_string(CHARACTER_TABLE, context->id, &result,
	CHARACTER_KEY_MAP, nullptr) == RET_OK)
	{
		free(context->map);
		ret = _context_set_map(context, result);
		free(result);
	}
	else
	{
		ret = RET_NOK;
	}

	int pos_tx;
	if (entry_read_int(CHARACTER_TABLE, context->id, &pos_tx,
	CHARACTER_KEY_TILE_X, nullptr) == RET_NOK)
	{
		ret = RET_NOK;
	}
	_context_set_pos_tx(context, pos_tx);

	int pos_ty;
	if (entry_read_int(CHARACTER_TABLE, context->id, &pos_ty,
	CHARACTER_KEY_TILE_Y, nullptr) == RET_NOK)
	{
		ret = RET_NOK;
	}
	_context_set_pos_ty(context, pos_ty);

	context_unlock_list();
	return ret;
}

/*******************************
 Write a context to server's disk
 return RET_NOK on error
 *******************************/
ret_code_t context_write_to_file(context_t * context)
{
	context_lock_list();

	if (context->id == nullptr)
	{
		context_unlock_list();
		return RET_NOK;
	}

	entry_write_string(CHARACTER_TABLE, context->id, context->type,
	CHARACTER_KEY_TYPE, nullptr);
	entry_write_string(CHARACTER_TABLE, context->id, context->map,
	CHARACTER_KEY_MAP, nullptr);

	entry_write_int(CHARACTER_TABLE, context->id, context->tile_x,
	CHARACTER_KEY_TILE_X, nullptr);

	entry_write_int(CHARACTER_TABLE, context->id, context->tile_y,
	CHARACTER_KEY_TILE_Y, nullptr);

	context_unlock_list();
	return RET_OK;
}

/*******************************
 Find a context in memory from its id
 *******************************/
context_t * context_find(const char * id)
{
	context_t * ctx;

	ctx = context_list_start;

	while (ctx != nullptr)
	{
		if (ctx->id)
		{
			if (strcmp(ctx->id, id) == 0)
			{
				return ctx;
			}
		}

		ctx = ctx->next;
	}

	return nullptr;
}

/**************************************
 Called from client
 **************************************/
void context_add_or_update_from_network_frame(const ContextBis & context)
{
	// search for this context
	context_lock_list();
	context_t * ctx = context_list_start;

	while (ctx != nullptr)
	{
		if (strcmp(context.getId().c_str(), ctx->id) == 0)
		{
			ctx->in_game = context.isInGame();
			ctx->connected = context.isConnected();

			if (context.isInGame() == true)
			{
				wlog(LOGDEVELOPER, "Updating context %s / %s", context.getUserName().c_str(), context.getCharacterName().c_str());
				// do not call context_set_* function since we already have the lock
				_context_set_map(ctx, context.getMap().c_str());

				_context_set_npc(ctx, context.isNpc());

				_context_set_pos_tx(ctx, context.getTileX());
				_context_set_pos_ty(ctx, context.getTileY());

				free(ctx->type);
				ctx->type = strdup(context.getType().c_str());

				ctx->selection = context.getSelection();
			}

			if (context.isConnected() == false)
			{
				wlog(LOGDEVELOPER, "Deleting context %s / %s", context.getUserName().c_str(), context.getCharacterName().c_str());
				context_free(ctx);
			}
			context_unlock_list();

			return;
		}
		ctx = ctx->next;
	}

	context_unlock_list();

	wlog(LOGDEVELOPER, "Creating context %s / %s", context.getUserName().c_str(), context.getCharacterName().c_str());
	ctx = context_new();
	context_set_username(ctx, context.getUserName().c_str());
	context_set_character_name(ctx, context.getCharacterName().c_str());
	context_set_npc(ctx, context.isNpc());
	context_set_map(ctx, context.getMap().c_str());
	context_set_type(ctx, context.getType().c_str());
	context_set_pos_tx(ctx, context.getTileX());
	context_set_pos_ty(ctx, context.getTileY());
	context_set_id(ctx, context.getId().c_str());
	context_set_connected(ctx, context.isConnected());
	context_set_in_game(ctx, context.isInGame());
	context_set_selected_character(ctx, context.getSelection().getId().c_str());
	context_set_selected_tile(ctx, context.getSelection().getMap().c_str(), context.getSelection().getMapCoordTx(), context.getSelection().getMapCoordTy());
	context_set_selected_equipment(ctx, context.getSelection().getEquipment().c_str());
	context_set_selected_item(ctx, context.getSelection().getInventory().c_str());
}

/**************************************
 Return the distance between two contexts
 **************************************/
int context_distance(context_t * ctx1, context_t * ctx2)
{
	int distx;
	int disty;

	distx = ctx1->tile_x - ctx2->tile_x;
	if (distx < 0)
	{
		distx = -distx;
	}
	disty = ctx1->tile_y - ctx2->tile_y;
	if (disty < 0)
	{
		disty = -disty;
	}

	return (distx > disty ? distx : disty);
}

/**************************************
 Return true is context is an NPC
 **************************************/
bool context_is_npc(context_t * ctx)
{
	if (ctx->socket == nullptr && ctx->connected == true)
	{
		return true;
	}

	return false;
}
