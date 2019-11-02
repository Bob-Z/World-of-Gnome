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

#include "../server/action.h"
#include "Context.h"
#include "entry.h"
#include "log.h"
#include "mutex.h"
#include "SdlLocking.h"
#include "syntax.h"

extern "C"
{
#include "lualib.h"
#include "lauxlib.h"
}

Context * context_list_start = nullptr;

/*****************************************************************************/
Context::Context() :
		m_mutex(nullptr), m_userName(), m_connected(false), m_inGame(false), m_npc(true), m_characterName(), m_map(), m_previousMap(), m_mapChanged(false), m_tileX(
				0), m_tileY(0), m_previousTileX(0), m_previousTileY(0), m_positionChanged(false), m_orientation(0), m_direction(0), m_animationTick(0), m_type(), m_id(), m_selection(), m_nextExecutionTick(
				0), m_socket(), m_socket_data(), m_send_mutex(nullptr), m_hostname(nullptr), m_lua_VM(nullptr), m_condition(nullptr), m_condition_mutex(
				nullptr), m_previous(nullptr), m_next(nullptr)
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
/************************************
 context_init
 Initialize a context
 *************************************/
void context_init(Context * context)
{
	context->m_socket = 0;
	context->m_socket_data = 0;
	context->m_hostname = nullptr;
	context->m_send_mutex = SDL_CreateMutex();

	context->m_lua_VM = nullptr;
	context->m_condition = nullptr;
	context->m_condition_mutex = nullptr;

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
		if (context->getId() == ctx->getSelectionContextId())
		{
			ctx->setSelectionContextId("");
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
 Returns false if error
 **************************/
bool context_set_hostname(Context * context, const char * name)
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
		return false;
	}

	context_unlock_list();
	return true;
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
 Return false if there is an error
 *******************************/
bool context_update_from_file(Context * context)
{
	// Don't call context_set_* functions here to avoid inter-blocking

	char * result;
	bool ret = false;

	context_lock_list();

	if (context->getId() == "")
	{
		context_unlock_list();
		return false;
	}

	if (entry_read_string(CHARACTER_TABLE, context->getId().c_str(), &result,
	CHARACTER_KEY_NAME, nullptr) == true)
	{
		context->setCharacterName(std::string(result));
	}
	else
	{
		ret = false;
	}

	int npc = 0;
	if (entry_read_int(CHARACTER_TABLE, context->getId().c_str(), &npc, CHARACTER_KEY_NPC, nullptr) == false)
	{
		ret = false;
	}
	context->setNpc(npc);

	if (entry_read_string(CHARACTER_TABLE, context->getId().c_str(), &result,
	CHARACTER_KEY_TYPE, nullptr) == false)
	{
		context->setType(std::string(result));
	}
	else
	{
		ret = false;
	}

	if (entry_read_string(CHARACTER_TABLE, context->getId().c_str(), &result,
	CHARACTER_KEY_MAP, nullptr) == true)
	{
		context->setMap(std::string(result));
		free(result);
	}
	else
	{
		ret = false;
	}

	int pos_tx;
	if (entry_read_int(CHARACTER_TABLE, context->getId().c_str(), &pos_tx,
	CHARACTER_KEY_TILE_X, nullptr) == false)
	{
		ret = false;
	}
	context->setTileX(pos_tx);

	int pos_ty;
	if (entry_read_int(CHARACTER_TABLE, context->getId().c_str(), &pos_ty,
	CHARACTER_KEY_TILE_Y, nullptr) == false)
	{
		ret = false;
	}
	context->setTileY(pos_ty);

	context_unlock_list();
	return ret;
}

/*******************************
 Write a context to server's disk
 return false on error
 *******************************/
int context_write_to_file(Context * context)
{
	context_lock_list();

	if (context->getId() == "")
	{
		context_unlock_list();
		return false;
	}

	entry_write_string(CHARACTER_TABLE, context->getId().c_str(), context->getType().c_str(),
	CHARACTER_KEY_TYPE, nullptr);
	entry_write_string(CHARACTER_TABLE, context->getId().c_str(), context->getMap().c_str(),
	CHARACTER_KEY_MAP, nullptr);

	entry_write_int(CHARACTER_TABLE, context->getId().c_str(), context->getTileX(),
	CHARACTER_KEY_TILE_X, nullptr);

	entry_write_int(CHARACTER_TABLE, context->getId().c_str(), context->getTileY(),
	CHARACTER_KEY_TILE_Y, nullptr);

	context_unlock_list();
	return true;
}

/*******************************
 Find a context in memory from its id
 *******************************/
Context * context_find(const std::string & id)
{
	Context * ctx;

	ctx = context_list_start;

	while (ctx != nullptr)
	{
		if (ctx->getId() == id)
		{
			return ctx;
		}

		ctx = ctx->m_next;
	}

	return nullptr;
}

/**************************************
 Called from client
 **************************************/
void context_add_or_update_from_network_frame(const Context & receivedCtx)
{
	context_lock_list();
	Context * ctx = context_list_start;

	while (ctx != nullptr)
	{
		if (receivedCtx.getId() == ctx->getId())
		{
			ctx->setInGame(receivedCtx.isInGame());
			ctx->setConnected(receivedCtx.isConnected());

			if (receivedCtx.isInGame() == true)
			{
				wlog(LOGDEVELOPER, "Updating context %s / %s", receivedCtx.getUserName().c_str(), receivedCtx.getCharacterName().c_str());

				ctx->setMap(receivedCtx.getMap());
				ctx->setTileX(receivedCtx.getTileX());
				ctx->setTileY(receivedCtx.getTileY());

				ctx->setNpc(receivedCtx.isNpc());

				ctx->setType(receivedCtx.getType());

				ctx->setSelection(receivedCtx.getSelection());
			}

			if (receivedCtx.isConnected() == false)
			{
				wlog(LOGDEVELOPER, "Deleting context %s / %s", receivedCtx.getUserName().c_str(), receivedCtx.getCharacterName().c_str());
				context_free(ctx);
			}
			context_unlock_list();

			return;
		}
		ctx = ctx->m_next;
	}

	context_unlock_list();

	wlog(LOGDEVELOPER, "Creating context %s / %s", receivedCtx.getUserName().c_str(), receivedCtx.getCharacterName().c_str());
	ctx = context_new();
	ctx->setUserName(receivedCtx.getUserName());
	ctx->setCharacterName(receivedCtx.getCharacterName().c_str());
	ctx->setNpc(receivedCtx.isNpc());
	ctx->setMap(receivedCtx.getMap());
	ctx->setType(receivedCtx.getType());
	ctx->setTileX(receivedCtx.getTileX());
	ctx->setTileY(receivedCtx.getTileY());
	ctx->setId(receivedCtx.getId());
	ctx->setConnected(receivedCtx.isConnected());
	ctx->setInGame(receivedCtx.isInGame());
	ctx->setSelectionContextId(receivedCtx.getSelection().getContextId());
	ctx->setSelectionTile(receivedCtx.getSelection().getMap(), receivedCtx.getSelection().getMapTx(), receivedCtx.getSelection().getMapTy());
	ctx->setSelectionEquipment(receivedCtx.getSelection().getEquipment());
	ctx->setSelectionInventory(receivedCtx.getSelection().getInventory());
}

/**************************************
 Return the distance between this context and the given context (in tile)
 **************************************/
int Context::tileDistance(const Context & ctx) const
{
	int distx = 0;
	int disty = 0;

	SdlLocking lock(m_mutex);

	distx = m_tileX - ctx.getTileX();
	if (distx < 0)
	{
		distx = -distx;
	}
	disty = m_tileY - ctx.getTileY();
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
	SdlLocking lock(m_mutex);

	return m_type;
}

/*****************************************************************************/
void Context::setType(const std::string& type)
{
	SdlLocking lock(m_mutex);

	m_type = type;
}

/*****************************************************************************/
const std::string& Context::getId() const
{
	SdlLocking lock(m_mutex);

	return m_id;
}

/*****************************************************************************/
void Context::setId(const std::string& id)
{
	SdlLocking lock(m_mutex);

	m_id = id;
}

/*****************************************************************************/
const Selection& Context::getSelection() const
{
	SdlLocking lock(m_mutex);

	return m_selection;
}

/*****************************************************************************/
void Context::setSelection(const Selection& selection)
{
	SdlLocking lock(m_mutex);

	m_selection = selection;
}

/*****************************************************************************/
void Context::setSelectionContextId(const std::string & id)
{
	SdlLocking lock(m_mutex);

	m_selection.setContextId(id);
}

/*****************************************************************************/
const std::string& Context::getSelectionContextId() const
{
	SdlLocking lock(m_mutex);

	return m_selection.getContextId();
}

/*****************************************************************************/
void Context::setSelectionTile(const std::string & map, int tx, int ty)
{
	SdlLocking lock(m_mutex);

	m_selection.setMap(map);
	m_selection.setMapTx(tx);
	m_selection.setMapTy(ty);
}

/*****************************************************************************/
const std::string& Context::getSelectionMap() const
{
	SdlLocking lock(m_mutex);

	return m_selection.getMap();
}

/*****************************************************************************/
int Context::getSelectionMapTx() const
{
	SdlLocking lock(m_mutex);
	return m_selection.getMapTx();
}

/*****************************************************************************/
int Context::getSelectionMapTy() const
{
	SdlLocking lock(m_mutex);
	return m_selection.getMapTy();
}

/*****************************************************************************/
void Context::setSelectionEquipment(const std::string & equipment)
{
	SdlLocking lock(m_mutex);
	m_selection.setEquipment(equipment);
}

/*****************************************************************************/
const std::string & Context::getSelectionEquipment() const
{
	SdlLocking lock(m_mutex);

	return m_selection.getEquipment();
}

/*****************************************************************************/
void Context::setSelectionInventory(const std::string & inventory)
{
	SdlLocking lock(m_mutex);

	m_selection.setInventory(inventory);
}

/*****************************************************************************/
const std::string & Context::getSelectionInventory() const
{
	SdlLocking lock(m_mutex);

	return m_selection.getInventory();
}

/*****************************************************************************/
Uint32 Context::getNextExecutionTick() const
{
	return m_nextExecutionTick;
}

/*****************************************************************************/
void Context::setNextExecutionTick(Uint32 nextExecutionTick)
{
	m_nextExecutionTick = nextExecutionTick;
}
