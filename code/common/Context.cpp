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

#include "Context.h"
#include "ContextBis.h"
#include "entry.h"
#include "log.h"
#include "mutex.h"
#include "SdlLocking.h"
#include "syntax.h"
#include <cstring>
#include <stdlib.h>
#include <string>

#include "../server/action.h"

class ContextBis;

extern "C"
{
#include "lualib.h"
#include "lauxlib.h"
}
#include <unistd.h>
#include <limits.h>
#include "common.h"

Context * context_list_start = nullptr;

/*****************************************************************************/
Context::Context() :
		m_mutex(nullptr), m_userName(), m_isConnected(false), m_in_game(false), m_socket(), m_socket_data(), m_send_mutex(nullptr), m_hostname(nullptr), m_render(
				nullptr), m_window(nullptr), m_npc(true), m_character_name(nullptr), m_map(nullptr), m_tile_x(-1), m_tile_y(-1), m_prev_pos_tile_x(-1), m_prev_pos_tile_y(
				-1), m_pos_changed(false), m_animation_tick(0), m_type(nullptr), m_selection(), m_id(nullptr), m_prev_map(nullptr), m_change_map(false), m_lua_VM(
				nullptr), m_condition(nullptr), m_condition_mutex(nullptr), m_orientation(0), m_direction(0), m_next_execution_time(0), m_previous(nullptr), m_next(
				nullptr)
{
	m_mutex = SDL_CreateMutex();
}

/*****************************************************************************/
Context::~Context()
{
	SDL_DestroyMutex(m_mutex);
}

/***********************
 ***********************/
Context * context_get_list_start()
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
Context * context_get_first()
{
	return context_list_start;
}
/*************************************
 context_init
 Initialize a context_t struct
 *************************************/
void context_init(Context * context)
{
	context->m_isConnected = false;
	context->m_in_game = false;
	context->m_socket = 0;
	context->m_socket_data = 0;
	context->m_hostname = nullptr;
	context->m_send_mutex = SDL_CreateMutex();

	context->m_render = nullptr;
	context->m_window = nullptr;

	context->m_npc = 1;
	context->m_character_name = nullptr;
	context->m_map = nullptr;

	context->m_tile_x = 0;
	context->m_tile_y = 0;
	context->m_prev_pos_tile_x = 0;
	context->m_prev_pos_tile_y = 0;
	context->m_pos_changed = false;
	context->m_animation_tick = 0;
	context->m_type = nullptr;

	context->m_id = nullptr;
	context->m_prev_map = nullptr;
	context->m_change_map = false;
	context->m_lua_VM = nullptr;
	context->m_condition = nullptr;
	context->m_condition_mutex = nullptr;
	context->m_orientation = 0;
	context->m_direction = 0;
	context->m_next_execution_time = 0;

	context->m_previous = nullptr;
	context->m_next = nullptr;
}

/**************************************
 Add a new context to the list
 **************************************/
Context * context_new(void)
{
	Context * ctx;

	context_lock_list();
	if (context_list_start == nullptr)
	{
		context_list_start = new Context;
		context_init(context_list_start);
		context_unlock_list();
		return context_list_start;
	}

	ctx = context_list_start;
	while (ctx->m_next != nullptr)
	{
		ctx = ctx->m_next;
	}

	ctx->m_next = new Context;
	context_init(ctx->m_next);
	ctx->m_next->m_previous = ctx;
	context_unlock_list();
	return ctx->m_next;
}
/*************************************
 context_free_data
 Deep free of all context_t data
 *************************************/
void context_free_data(Context * context)
{
	context->m_in_game = false;
	context->m_isConnected = false;
	if (context->m_socket != 0)
	{
		SDLNet_TCP_Close(context->m_socket);
	}
	context->m_socket = 0;
	if (context->m_socket_data != 0)
	{
		SDLNet_TCP_Close(context->m_socket_data);
	}
	context->m_socket_data = 0;
	SDL_DestroyMutex(context->m_send_mutex);

	if (context->m_hostname)
	{
		free(context->m_hostname);
	}
	context->m_hostname = nullptr;
	if (context->m_character_name)
	{
		free(context->m_character_name);
	}
	context->m_character_name = nullptr;
	if (context->m_map)
	{
		free(context->m_map);
	}
	context->m_map = nullptr;
	if (context->m_type)
	{
		free(context->m_type);
	}
	context->m_type = nullptr;

	context->m_prev_map = nullptr;
	if (context->m_lua_VM != nullptr)
	{
		lua_close(context->m_lua_VM);
	}
	if (context->m_condition != nullptr)
	{
		SDL_DestroyCond(context->m_condition);
	}
	if (context->m_condition_mutex != nullptr)
	{
		SDL_DestroyMutex(context->m_condition_mutex);
	}

	if (context->m_id)
	{
		free(context->m_id);
	}
	context->m_id = nullptr;
}

/*************************************
 context_free
 Deep free of a context_t struct and remove it from context list
 *************************************/
void context_free(Context * context)
{
	Context * ctx;

	context_lock_list();

	// First context of the list
	if (context->m_previous == nullptr)
	{
		context_list_start = context->m_next;
		if (context->m_next != nullptr)
		{
			context->m_next->m_previous = nullptr;
		}
	}
	else
	{
		context->m_previous->m_next = context->m_next;
		if (context->m_next != nullptr)
		{
			context->m_next->m_previous = context->m_previous;
		}
	}

	// Remove this context from other context's selection
	ctx = context_list_start;
	while (ctx != nullptr)
	{
		if ((context->m_id != nullptr) && (context->m_selection.getId() != ""))
		{
			if (strcmp(context->m_id, ctx->m_selection.getId().c_str()) == 0)
			{
				ctx->m_selection.setId("");
			}
		}
		ctx = ctx->m_next;
	}

	// Remove this context from the list
	if (context == context_get_first())
	{
		context_list_start = context->m_next;
	}
	else
	{
		ctx = context_list_start;
		while (ctx != nullptr)
		{
			if (ctx->m_next == context)
			{
				ctx->m_next = context->m_next;
				break;
			}
			ctx = ctx->m_next;
		}
	}

	context_unlock_list();

	context_free_data(context);

	delete (context);
}

/***********************
 ***********************/
void context_lock_list()
{
	SDL_LockMutex(context_list_mutex);
}

/***********************
 ***********************/
Context * context_get_player()
{
	return context_list_start;
}
/**************************
 Returns RET_NOK if error
 **************************/
ret_code_t context_set_hostname(Context * context, const char * name)
{
	context_lock_list();

	if (context->m_hostname)
	{
		free(context->m_hostname);
	}

	context->m_hostname = strdup(name);
	if (context->m_hostname == nullptr)
	{
		context_unlock_list();
		return RET_NOK;
	}

	context_unlock_list();
	return RET_OK;
}

/**************************************
 **************************************/
void context_set_in_game(Context * context, bool in_game)
{
	context_lock_list();
	context->m_in_game = in_game;
	context_unlock_list();
}

/**************************************
 **************************************/
int context_get_in_game(Context * context)
{
	int in_game = 0;

	context_lock_list();
	in_game = context->m_in_game;
	context_unlock_list();

	return in_game;
}

/**************************************
 **************************************/
void context_set_connected(Context * context, bool connected)
{
	context_lock_list();
	context->m_isConnected = connected;
	context_unlock_list();
}

/**************************************
 **************************************/
int context_get_connected(Context * context)
{
	int conn = 0;

	context_lock_list();
	conn = context->m_isConnected;
	context_unlock_list();

	return conn;
}

/**************************************
 **************************************/
void context_set_socket(Context * context, TCPsocket socket)
{
	context_lock_list();
	context->m_socket = socket;
	context_unlock_list();
}

/**************************************
 **************************************/
TCPsocket context_get_socket(Context * context)
{
	TCPsocket socket = 0;

	context_lock_list();
	socket = context->m_socket;
	context_unlock_list();

	return socket;
}

/**************************************
 **************************************/
void context_set_socket_data(Context * context, TCPsocket socket)
{
	context_lock_list();
	context->m_socket_data = socket;
	context_unlock_list();
}

/**************************************
 **************************************/
TCPsocket context_get_socket_data(Context * context)
{
	TCPsocket socket = 0;

	context_lock_list();
	socket = context->m_socket_data;
	context_unlock_list();

	return socket;
}

/**************************************
 Returns RET_NOK if error
 **************************************/
ret_code_t context_set_character_name(Context * context, const char * name)
{
	ret_code_t ret = RET_OK;

	context_lock_list();
	free(context->m_character_name);
	context->m_character_name = strdup(name);
	if (context->m_character_name == nullptr)
	{
		ret = RET_NOK;
	}
	context_unlock_list();

	return ret;
}

/**************************************
 Returns RET_NOK if error
 **************************************/
static ret_code_t _context_set_map(Context * context, const char * map)
{
	if (context->m_prev_map != nullptr)
	{
		if (!strcmp(context->m_map, map))
		{
			return RET_OK;
		}
		free(context->m_prev_map);
		context->m_prev_map = nullptr;
	}

	if (context->m_map)
	{
		context->m_prev_map = context->m_map;
	}

	context->m_map = strdup(map);
	if (context->m_map == nullptr)
	{
		return RET_NOK;
	}

	context->m_change_map = true;

	return RET_OK;
}

/**************************************
 **************************************/
ret_code_t context_set_map(Context * context, const char * map)
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
ret_code_t context_set_type(Context * context, const char * type)
{
	ret_code_t ret = RET_OK;

	context_lock_list();
	free(context->m_type);
	context->m_type = strdup(type);
	if (context->m_type == nullptr)
	{
		ret = RET_NOK;
	}
	context_unlock_list();

	return ret;
}

/**************************************
 **************************************/
void _context_set_npc(Context * context, bool npc)
{
	context->m_npc = npc;
}

/**************************************
 **************************************/
void context_set_npc(Context * context, bool npc)
{
	context_lock_list();
	_context_set_npc(context, npc);
	context_unlock_list();
}

/**************************************
 **************************************/
void _context_set_pos_tx(Context * context, int pos_tx)
{
	context->m_prev_pos_tile_x = context->m_tile_x;
	if (context->m_tile_x != pos_tx)
	{
		context->m_pos_changed = true;
		context->m_tile_x = pos_tx;
	}
}

/**************************************
 **************************************/
void context_set_pos_tx(Context * context, int pos_tx)
{
	context_lock_list();
	_context_set_pos_tx(context, pos_tx);
	context_unlock_list();
}
/**************************************
 **************************************/
void _context_set_pos_ty(Context * context, int pos_ty)
{
	context->m_prev_pos_tile_y = context->m_tile_y;
	if (context->m_tile_y != pos_ty)
	{
		context->m_pos_changed = true;
		context->m_tile_y = pos_ty;
	}
}

/**************************************
 **************************************/
void context_set_pos_ty(Context * context, int pos_ty)
{
	context_lock_list();
	_context_set_pos_ty(context, pos_ty);
	context_unlock_list();
}

/**************************************
 return RET_NOK on error
 **************************************/
ret_code_t context_set_id(Context * context, const char * name)
{
	context_lock_list();

	if (context->m_id)
	{
		free(context->m_id);
	}
	context->m_id = strdup(name);
	if (context->m_id == nullptr)
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
ret_code_t context_set_selected_character(Context * context, const char * id)
{
	context_lock_list();

	if ((context != nullptr) && (context->m_selection.getId() != ""))
	{
		if (strcmp(context->m_selection.getId().c_str(), id) == 0)
		{
			context_unlock_list();
			return RET_NOK;
		}
		context->m_selection.setId(std::string(id));
	}

	context_unlock_list();
	return RET_OK;
}

/**************************************
 * return RET_NOK if context has already the same values
 **************************************/
ret_code_t context_set_selected_tile(Context * context, const char * map, int x, int y)
{
	context_lock_list();

	if (!strcmp(context->m_selection.getMap().c_str(), map))
	{
		if ((x == context->m_selection.getMapCoordTx()) && (y == context->m_selection.getMapCoordTy()))
		{
			context_unlock_list();
			return RET_NOK;
		}
	}

	context->m_selection.setMap(std::string(map));
	context->m_selection.setMapCoordTx(x);
	context->m_selection.setMapCoordTy(y);

	context_unlock_list();
	return RET_OK;
}

/**************************************
 * return RET_NOK if context has already the same values
 **************************************/
ret_code_t context_set_selected_equipment(Context * context, const char * id)
{
	context_lock_list();

	if (strcmp(context->m_selection.getEquipment().c_str(), id) == 0)
	{
		context_unlock_list();
		return RET_NOK;
	}

	context->m_selection.setEquipment(id);

	context_unlock_list();
	return RET_OK;
}

/**************************************
 * return RET_NOK if context has already the same values
 **************************************/
ret_code_t context_set_selected_item(Context * context, const char * id)
{
	context_lock_list();

	if (strcmp(context->m_selection.getInventory().c_str(), id) == 0)
	{
		context_unlock_list();
		return RET_NOK;
	}

	context->m_selection.setInventory(id);

	context_unlock_list();
	return RET_OK;
}

/**************************************
 **************************************/
// Client use only one LUA VM, not one LUA VM by context
#ifdef SERVER
void register_lua_functions(Context * context);
#endif

/**************************************
 **************************************/
void context_new_VM(Context * context)
{
	context_lock_list();
	context->m_lua_VM = lua_open();
	lua_baselibopen(context->m_lua_VM);
	lua_tablibopen(context->m_lua_VM);
	lua_iolibopen(context->m_lua_VM);
	lua_strlibopen(context->m_lua_VM);
	lua_mathlibopen(context->m_lua_VM);

	register_lua_functions(context);
	context_unlock_list();
}

/*******************************
 Update the memory context by reading the character's data file on disk
 Return RET_NOK if there is an error
 *******************************/
ret_code_t context_update_from_file(Context * context)
{
	// Don't call context_set_* functions here to avoid inter-blocking

	char * result;
	ret_code_t ret = RET_OK;

	context_lock_list();

	if (context->m_id == nullptr)
	{
		context_unlock_list();
		return RET_NOK;
	}

	if (entry_read_string(CHARACTER_TABLE, context->m_id, &result,
	CHARACTER_KEY_NAME, nullptr) == RET_OK)
	{
		free(context->m_character_name);
		context->m_character_name = result;
	}
	else
	{
		ret = RET_NOK;
	}

	int npc = 0;
	if (entry_read_int(CHARACTER_TABLE, context->m_id, &npc, CHARACTER_KEY_NPC, nullptr) == RET_NOK)
	{
		ret = RET_NOK;
	}
	_context_set_npc(context, npc);

	if (entry_read_string(CHARACTER_TABLE, context->m_id, &result,
	CHARACTER_KEY_TYPE, nullptr) == RET_OK)
	{
		free(context->m_type);
		context->m_type = result;
	}
	else
	{
		ret = RET_NOK;
	}

	if (entry_read_string(CHARACTER_TABLE, context->m_id, &result,
	CHARACTER_KEY_MAP, nullptr) == RET_OK)
	{
		free(context->m_map);
		ret = _context_set_map(context, result);
		free(result);
	}
	else
	{
		ret = RET_NOK;
	}

	int pos_tx;
	if (entry_read_int(CHARACTER_TABLE, context->m_id, &pos_tx,
	CHARACTER_KEY_TILE_X, nullptr) == RET_NOK)
	{
		ret = RET_NOK;
	}
	_context_set_pos_tx(context, pos_tx);

	int pos_ty;
	if (entry_read_int(CHARACTER_TABLE, context->m_id, &pos_ty,
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
ret_code_t context_write_to_file(Context * context)
{
	context_lock_list();

	if (context->m_id == nullptr)
	{
		context_unlock_list();
		return RET_NOK;
	}

	entry_write_string(CHARACTER_TABLE, context->m_id, context->m_type,
	CHARACTER_KEY_TYPE, nullptr);
	entry_write_string(CHARACTER_TABLE, context->m_id, context->m_map,
	CHARACTER_KEY_MAP, nullptr);

	entry_write_int(CHARACTER_TABLE, context->m_id, context->m_tile_x,
	CHARACTER_KEY_TILE_X, nullptr);

	entry_write_int(CHARACTER_TABLE, context->m_id, context->m_tile_y,
	CHARACTER_KEY_TILE_Y, nullptr);

	context_unlock_list();
	return RET_OK;
}

/*******************************
 Find a context in memory from its id
 *******************************/
Context * context_find(const char * id)
{
	Context * ctx;

	ctx = context_list_start;

	while (ctx != nullptr)
	{
		if (ctx->m_id)
		{
			if (strcmp(ctx->m_id, id) == 0)
			{
				return ctx;
			}
		}

		ctx = ctx->m_next;
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
	Context * ctx = context_list_start;

	while (ctx != nullptr)
	{
		if (strcmp(context.getId().c_str(), ctx->m_id) == 0)
		{
			ctx->m_in_game = context.isInGame();
			ctx->m_isConnected = context.isConnected();

			if (context.isInGame() == true)
			{
				wlog(LOGDEVELOPER, "Updating context %s / %s", context.getUserName().c_str(), context.getCharacterName().c_str());
				// do not call context_set_* function since we already have the lock
				_context_set_map(ctx, context.getMap().c_str());

				_context_set_npc(ctx, context.isNpc());

				_context_set_pos_tx(ctx, context.getTileX());
				_context_set_pos_ty(ctx, context.getTileY());

				free(ctx->m_type);
				ctx->m_type = strdup(context.getType().c_str());

				ctx->m_selection = context.getSelection();
			}

			if (context.isConnected() == false)
			{
				wlog(LOGDEVELOPER, "Deleting context %s / %s", context.getUserName().c_str(), context.getCharacterName().c_str());
				context_free(ctx);
			}
			context_unlock_list();

			return;
		}
		ctx = ctx->m_next;
	}

	context_unlock_list();

	wlog(LOGDEVELOPER, "Creating context %s / %s", context.getUserName().c_str(), context.getCharacterName().c_str());
	ctx = context_new();
	ctx->setUserName(context.getUserName());
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
int context_distance(Context * ctx1, Context * ctx2)
{
	int distx;
	int disty;

	distx = ctx1->m_tile_x - ctx2->m_tile_x;
	if (distx < 0)
	{
		distx = -distx;
	}
	disty = ctx1->m_tile_y - ctx2->m_tile_y;
	if (disty < 0)
	{
		disty = -disty;
	}

	return (distx > disty ? distx : disty);
}

/**************************************
 Return true is context is an NPC
 **************************************/
bool context_is_npc(Context * ctx)
{
	if (ctx->m_socket == nullptr && ctx->m_isConnected == true)
	{
		return true;
	}

	return false;
}

/*****************************************************************************/
const std::string& Context::getUserName() const
{
	SdlLocking lock(m_mutex);

	return m_userName;
}

/*****************************************************************************/
void Context::setUserName(const std::string& userName)
{
	SdlLocking lock(m_mutex);

	m_userName = userName;
}
