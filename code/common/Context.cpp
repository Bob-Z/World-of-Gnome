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
		m_mutex(nullptr), m_userName(), m_connected(false), m_inGame(false), m_npc(true), m_characterName(), m_map(), m_previousMap(), m_mapChanged(false), m_tileX(
				0), m_tileY(0), m_previousTileX(0), m_previousTileY(0), m_positionChanged(false), m_orientation(0), m_direction(0), m_animationTick(0), m_type(), m_socket(), m_socket_data(), m_send_mutex(
				nullptr), m_hostname(nullptr), m_render(nullptr), m_window(nullptr), m_selection(), m_id(nullptr), m_lua_VM(nullptr), m_condition(nullptr), m_condition_mutex(
				nullptr), m_next_execution_time(0), m_previous(nullptr), m_next(nullptr)
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
	context->m_socket = 0;
	context->m_socket_data = 0;
	context->m_hostname = nullptr;
	context->m_send_mutex = SDL_CreateMutex();

	context->m_render = nullptr;
	context->m_window = nullptr;

	context->m_id = nullptr;
	context->m_lua_VM = nullptr;
	context->m_condition = nullptr;
	context->m_condition_mutex = nullptr;
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
		context->setCharacterName(std::string(result));
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
	context->setNpc(npc);

	if (entry_read_string(CHARACTER_TABLE, context->m_id, &result,
	CHARACTER_KEY_TYPE, nullptr) == RET_OK)
	{
		context->setType(std::string(result));
	}
	else
	{
		ret = RET_NOK;
	}

	if (entry_read_string(CHARACTER_TABLE, context->m_id, &result,
	CHARACTER_KEY_MAP, nullptr) == RET_OK)
	{
		context->setMap(std::string(result));
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
	context->setTileX(pos_tx);

	int pos_ty;
	if (entry_read_int(CHARACTER_TABLE, context->m_id, &pos_ty,
	CHARACTER_KEY_TILE_Y, nullptr) == RET_NOK)
	{
		ret = RET_NOK;
	}
	context->setTileY(pos_ty);

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

	entry_write_string(CHARACTER_TABLE, context->m_id, context->getType().c_str(),
	CHARACTER_KEY_TYPE, nullptr);
	entry_write_string(CHARACTER_TABLE, context->m_id, context->getMap().c_str(),
	CHARACTER_KEY_MAP, nullptr);

	entry_write_int(CHARACTER_TABLE, context->m_id, context->getTileX(),
	CHARACTER_KEY_TILE_X, nullptr);

	entry_write_int(CHARACTER_TABLE, context->m_id, context->getTileY(),
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
			ctx->setInGame(context.isInGame());
			ctx->setConnected(context.isConnected());

			if (context.isInGame() == true)
			{
				wlog(LOGDEVELOPER, "Updating context %s / %s", context.getUserName().c_str(), context.getCharacterName().c_str());
				// do not call context_set_* function since we already have the lock
				ctx->setMap(context.getMap());

				ctx->setNpc(context.isNpc());

				ctx->setTileX(context.getTileX());
				ctx->setTileY(context.getTileY());

				ctx->setType(context.getType());

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
	ctx->setCharacterName(context.getCharacterName().c_str());
	ctx->setNpc(context.isNpc());
	ctx->setMap(context.getMap());
	ctx->setType(context.getType());
	ctx->setTileX(context.getTileX());
	ctx->setTileY(context.getTileY());
	context_set_id(ctx, context.getId().c_str());
	ctx->setConnected(context.isConnected());
	ctx->setInGame(context.isInGame());
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

	distx = ctx1->getTileX() - ctx2->getTileX();
	if (distx < 0)
	{
		distx = -distx;
	}
	disty = ctx1->getTileY() - ctx2->getTileY();
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
	if (ctx->m_socket == nullptr && ctx->isConnected() == true)
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

/*****************************************************************************/
bool Context::isConnected() const
{
	return m_connected;
}

/*****************************************************************************/
void Context::setConnected(bool connected)
{
	m_connected = connected;
}

/*****************************************************************************/
bool Context::isInGame() const
{
	return m_inGame;
}

/*****************************************************************************/
void Context::setInGame(bool inGame)
{
	m_inGame = inGame;
}

/*****************************************************************************/
bool Context::isNpc() const
{
	return m_npc;
}

/*****************************************************************************/
void Context::setNpc(bool npc)
{
	m_npc = npc;
}

/*****************************************************************************/
const std::string& Context::getCharacterName() const
{
	SdlLocking lock(m_mutex);

	return m_characterName;
}

/*****************************************************************************/
void Context::setCharacterName(const std::string& characterName)
{
	SdlLocking lock(m_mutex);

	m_characterName = characterName;
}

/*****************************************************************************/
const std::string& Context::getMap() const
{
	SdlLocking lock(m_mutex);

	return m_map;
}

/*****************************************************************************/
void Context::setMap(const std::string& map)
{
	SdlLocking lock(m_mutex);

	m_previousMap = m_map;

	if (m_map != map)
	{
		m_mapChanged = true;
		m_map = map;
	}
}

/*****************************************************************************/
const std::string& Context::getPreviousMap() const
{
	SdlLocking lock(m_mutex);

	return m_previousMap;
}

/*****************************************************************************/
void Context::setPreviousMap(const std::string& previousMap)
{
	SdlLocking lock(m_mutex);

	m_previousMap = previousMap;
}

/*****************************************************************************/
bool Context::isMapChanged() const
{
	SdlLocking lock(m_mutex);

	return m_mapChanged;
}

/*****************************************************************************/
void Context::setMapChanged(bool mapChanged)
{
	SdlLocking lock(m_mutex);

	m_mapChanged = mapChanged;
}

/*****************************************************************************/
bool Context::isPositionChanged() const
{
	SdlLocking lock(m_mutex);

	return m_positionChanged;
}

/*****************************************************************************/
void Context::setPositionChanged(bool positionChanged)
{
	SdlLocking lock(m_mutex);

	m_positionChanged = positionChanged;
}

/*****************************************************************************/
int Context::getTileX() const
{
	SdlLocking lock(m_mutex);

	return m_tileX;
}

/*****************************************************************************/
void Context::setTileX(int tileX)
{
	SdlLocking lock(m_mutex);

	if (tileX != m_previousTileX)
	{
		m_previousTileX = m_tileX;
		m_tileX = tileX;
		m_positionChanged = true;
	}
}

/*****************************************************************************/
int Context::getTileY() const
{
	SdlLocking lock(m_mutex);

	return m_tileY;
}

/*****************************************************************************/
void Context::setTileY(int tileY)
{
	SdlLocking lock(m_mutex);

	if (tileY != m_previousTileY)
	{
		m_previousTileY = m_tileY;
		m_tileY = tileY;
		m_positionChanged = true;
	}
}

/*****************************************************************************/
int Context::getDirection() const
{
	return m_direction;
}

/*****************************************************************************/
void Context::setDirection(int direction)
{
	m_direction = direction;
}

/*****************************************************************************/
int Context::getOrientation() const
{
	return m_orientation;
}

/*****************************************************************************/
void Context::setOrientation(int orientation)
{
	m_orientation = orientation;
}

/*****************************************************************************/
int Context::getPreviousTileX() const
{
	return m_previousTileX;
}

/*****************************************************************************/
int Context::getPreviousTileY() const
{
	return m_previousTileY;
}

/*****************************************************************************/
Uint32 Context::getAnimationTick() const
{
	return m_animationTick;
}

/*****************************************************************************/
void Context::setAnimationTick(Uint32 animationTick)
{
	m_animationTick = animationTick;
}

/*****************************************************************************/
const std::string& Context::getType() const
{
	return m_type;
}

/*****************************************************************************/
void Context::setType(const std::string& type)
{
	m_type = type;
}
