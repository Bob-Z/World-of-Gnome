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

#include "LockGuard.h"
#include "Context.h"

#include "../server/action.h"
#include "entry.h"
#include "log.h"
#include "mutex.h"
#include "syntax.h"

extern "C"
{
#include "lualib.h"
#include "lauxlib.h"
}

Context *context_list_start = nullptr;

/*****************************************************************************/
Context::Context() :
		m_lock(), m_inGame(false), m_npc(true), m_characterName(), m_map(), m_previousMap(), m_mapChanged(
				false), m_tileX(0), m_tileY(0), m_previousTileX(0), m_previousTileY(
				0), m_positionChanged(false), m_orientation(0), m_direction(0), m_animationTick(
				0), m_type(), m_id(), m_selection(), m_nextExecutionTick(0), m_luaVm(
				nullptr), m_luaVmLock(), m_condition(SDL_CreateCond()), m_conditionLock(), m_connection(
				nullptr), m_npcThread(nullptr), m_actionRunning(), m_previous(
				nullptr), m_next(nullptr)
{
}

/*****************************************************************************/
Context::~Context()
{
	if (m_luaVm != nullptr)
	{
		lua_close(m_luaVm);
	}
	if (m_condition != nullptr)
	{
		SDL_DestroyCond(m_condition);
	}
}

/*****************************************************************************/
Context* context_get_list_start()
{
	return context_list_start;
}

/*****************************************************************************/
void context_unlock_list()
{
	context_list_lock.unlock();
}

/*****************************************************************************/
Context* context_get_first()
{
	return context_list_start;
}

/*****************************************************************************/
static void context_init(Context *context)
{
	context->m_previous = nullptr;
	context->m_next = nullptr;
}

/**************************************
 Add a new context to the list
 **************************************/
Context* context_new(void)
{
	Context *ctx;

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
 context_free
 Deep free of a context_t struct and remove it from context list
 *************************************/
void context_free(Context *context)
{
	Context *ctx = nullptr;

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

	delete (context);
}

/*****************************************************************************/
void context_lock_list()
{
	context_list_lock.lock();
}

/*****************************************************************************/
Context* context_get_player()
{
	return context_list_start;
}

/*******************************
 Write a context to server's disk
 return false on error
 *******************************/
int context_write_to_file(Context *context)
{
	context_lock_list();

	if (context->getId() == "")
	{
		context_unlock_list();
		return false;
	}

	entry_write_string(CHARACTER_TABLE.c_str(), context->getId().c_str(),
			context->getType().c_str(), CHARACTER_KEY_TYPE, nullptr);
	entry_write_string(CHARACTER_TABLE.c_str(), context->getId().c_str(),
			context->getMap().c_str(), CHARACTER_KEY_MAP, nullptr);

	entry_write_int(CHARACTER_TABLE.c_str(), context->getId().c_str(),
			context->getTileX(), CHARACTER_KEY_TILE_X, nullptr);

	entry_write_int(CHARACTER_TABLE.c_str(), context->getId().c_str(),
			context->getTileY(), CHARACTER_KEY_TILE_Y, nullptr);

	context_unlock_list();
	return true;
}

/*******************************
 Find a context in memory from its id
 *******************************/
Context* context_find(const std::string &id)
{
	Context *ctx;

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
void context_add_or_update_from_network_frame(const Context &receivedCtx)
{
	context_lock_list();
	Context *ctx = context_list_start;

	while (ctx != nullptr)
	{
		if (receivedCtx.getId() == ctx->getId())
		{
			ctx->setInGame(receivedCtx.isInGame());

			if (receivedCtx.isInGame() == true)
			{
				wlog(LOGDEVELOPER, "Updating context %s",
						receivedCtx.getCharacterName().c_str());

				ctx->setMap(receivedCtx.getMap());
				ctx->setTile(receivedCtx.getTileX(), receivedCtx.getTileY());

				ctx->setNpc(receivedCtx.isNpc());

				ctx->setType(receivedCtx.getType());

				ctx->setSelection(receivedCtx.getSelection());
			}

			if (receivedCtx.isInGame() == false)
			{
				wlog(LOGDEVELOPER, "Deleting context %s / %s",
						receivedCtx.getConnection()->getUserName().c_str(),
						receivedCtx.getCharacterName().c_str());
				context_free(ctx);
			}
			context_unlock_list();

			return;
		}
		ctx = ctx->m_next;
	}

	context_unlock_list();

	wlog(LOGDEVELOPER, "Creating context %s",
			receivedCtx.getCharacterName().c_str());
	ctx = context_new();
	ctx->setCharacterName(receivedCtx.getCharacterName().c_str());
	ctx->setNpc(receivedCtx.isNpc());
	ctx->setMap(receivedCtx.getMap());
	ctx->setType(receivedCtx.getType());
	ctx->setTile(receivedCtx.getTileX(), receivedCtx.getTileY());
	ctx->setId(receivedCtx.getId());
	ctx->setInGame(receivedCtx.isInGame());
	ctx->setSelectionContextId(receivedCtx.getSelection().getContextId());
	ctx->setSelectionTile(receivedCtx.getSelection().getMap(),
			receivedCtx.getSelection().getMapTx(),
			receivedCtx.getSelection().getMapTy());
	ctx->setSelectionEquipment(receivedCtx.getSelection().getEquipment());
	ctx->setSelectionInventory(receivedCtx.getSelection().getInventory());
}

/**************************************
 Return the distance between this context and the given context (in tile)
 **************************************/
int Context::tileDistance(const Context &ctx) const
{
	int distx = 0;
	int disty = 0;

	LockGuard guard(m_lock);

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
bool Context::isNpcActive() const
{
	LockGuard guard(m_lock);

	return ((m_npc == true) && (m_inGame == true));
}

/*****************************************************************************/
const std::string& Context::getCharacterName() const
{
	LockGuard guard(m_lock);

	return m_characterName;
}

/*****************************************************************************/
void Context::setCharacterName(const std::string &characterName)
{
	LockGuard guard(m_lock);

	m_characterName = characterName;
}

/*****************************************************************************/
const std::string& Context::getMap() const
{
	LockGuard guard(m_lock);

	return m_map;
}

/*****************************************************************************/
void Context::setMap(const std::string &map)
{
	LockGuard guard(m_lock);

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
	LockGuard guard(m_lock);

	return m_previousMap;
}

/*****************************************************************************/
void Context::setPreviousMap(const std::string &previousMap)
{
	LockGuard guard(m_lock);

	m_previousMap = previousMap;
}

/*****************************************************************************/
bool Context::isMapChanged() const
{
	LockGuard guard(m_lock);

	return m_mapChanged;
}

/*****************************************************************************/
void Context::setMapChanged(bool mapChanged)
{
	LockGuard guard(m_lock);

	m_mapChanged = mapChanged;
}

/*****************************************************************************/
bool Context::isPositionChanged() const
{
	LockGuard guard(m_lock);

	return m_positionChanged;
}

/*****************************************************************************/
void Context::setPositionChanged(bool positionChanged)
{
	LockGuard guard(m_lock);

	m_positionChanged = positionChanged;
}

/*****************************************************************************/
int Context::getTileX() const
{
	LockGuard guard(m_lock);

	return m_tileX;
}

/*****************************************************************************/
int Context::getTileY() const
{
	LockGuard guard(m_lock);

	return m_tileY;
}

/*****************************************************************************/
void Context::setTile(const int tileX, const int tileY)
{
	LockGuard guard(m_lock);

	if ((tileX != m_tileX) || (tileY != m_tileY))
	{
		m_previousTileX = m_tileX;
		m_previousTileY = m_tileY;
		m_tileX = tileX;
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
	LockGuard guard(m_lock);

	return m_type;
}

/*****************************************************************************/
void Context::setType(const std::string &type)
{
	LockGuard guard(m_lock);

	m_type = type;
}

/*****************************************************************************/
const std::string& Context::getId() const
{
	LockGuard guard(m_lock);

	return m_id;
}

/*****************************************************************************/
void Context::setId(const std::string &id)
{
	LockGuard guard(m_lock);

	m_id = id;
}

/*****************************************************************************/
const Selection& Context::getSelection() const
{
	LockGuard guard(m_lock);

	return m_selection;
}

/*****************************************************************************/
void Context::setSelection(const Selection &selection)
{
	LockGuard guard(m_lock);

	m_selection = selection;
}

/*****************************************************************************/
void Context::setSelectionContextId(const std::string &id)
{
	LockGuard guard(m_lock);

	m_selection.setContextId(id);
}

/*****************************************************************************/
const std::string& Context::getSelectionContextId() const
{
	LockGuard guard(m_lock);

	return m_selection.getContextId();
}

/*****************************************************************************/
void Context::setSelectionTile(const std::string &map, int tx, int ty)
{
	LockGuard guard(m_lock);

	m_selection.setMap(map);
	m_selection.setMapTx(tx);
	m_selection.setMapTy(ty);
}

/*****************************************************************************/
const std::string& Context::getSelectionMap() const
{
	LockGuard guard(m_lock);

	return m_selection.getMap();
}

/*****************************************************************************/
int Context::getSelectionMapTx() const
{
	LockGuard guard(m_lock);
	return m_selection.getMapTx();
}

/*****************************************************************************/
int Context::getSelectionMapTy() const
{
	LockGuard guard(m_lock);
	return m_selection.getMapTy();
}

/*****************************************************************************/
void Context::setSelectionEquipment(const std::string &equipment)
{
	LockGuard guard(m_lock);
	m_selection.setEquipment(equipment);
}

/*****************************************************************************/
const std::string& Context::getSelectionEquipment() const
{
	LockGuard guard(m_lock);

	return m_selection.getEquipment();
}

/*****************************************************************************/
void Context::setSelectionInventory(const std::string &inventory)
{
	LockGuard guard(m_lock);

	m_selection.setInventory(inventory);
}

/*****************************************************************************/
const std::string& Context::getSelectionInventory() const
{
	LockGuard guard(m_lock);

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

/*****************************************************************************/
lua_State* Context::getLuaVm()
{
	if (m_luaVm == nullptr)
	{
		m_luaVm = lua_open();
		lua_baselibopen(m_luaVm);
		lua_tablibopen(m_luaVm);
		lua_iolibopen(m_luaVm);
		lua_strlibopen(m_luaVm);
		lua_mathlibopen(m_luaVm);

		register_lua_functions(static_cast<Context*>(this));
	}

	return m_luaVm;
}

/*****************************************************************************/
void Context::wakeUp()
{
	if (m_conditionLock.trylock() == 0)
	{
		SDL_CondSignal(m_condition);
		m_conditionLock.unlock();
	}
}

/*****************************************************************************/
void Context::sleep(Uint32 timeOutMs)
{
	LockGuard guard(m_conditionLock);
	SDL_CondWaitTimeout(m_condition, m_conditionLock.getLock(), timeOutMs);
}

/*****************************************************************************/
bool Context::update_from_file()
{
	LockGuard guard(m_lock);

	if (getId() == "")
	{
		return false;
	}

	char *result = nullptr;
	bool ret = false;

	if (entry_read_string(CHARACTER_TABLE.c_str(), getId().c_str(), &result,
			CHARACTER_KEY_NAME, nullptr) == true)
	{
		setCharacterName(std::string(result));
	}
	else
	{
		ret = false;
	}

	int npc = 0;
	if (entry_read_int(CHARACTER_TABLE.c_str(), getId().c_str(), &npc,
			CHARACTER_KEY_NPC, nullptr) == false)
	{
		ret = false;
	}
	setNpc(npc);

	if (entry_read_string(CHARACTER_TABLE.c_str(), getId().c_str(), &result,
			CHARACTER_KEY_TYPE, nullptr) == true)
	{
		setType(std::string(result));
		free(result);
	}
	else
	{
		ret = false;
	}

	if (entry_read_string(CHARACTER_TABLE.c_str(), getId().c_str(), &result,
			CHARACTER_KEY_MAP, nullptr) == true)
	{
		setMap(std::string(result));
		free(result);
	}
	else
	{
		ret = false;
	}

	int pos_tx = 0;
	if (entry_read_int(CHARACTER_TABLE.c_str(), getId().c_str(), &pos_tx,
			CHARACTER_KEY_TILE_X, nullptr) == false)
	{
		ret = false;
	}
	int pos_ty = 0;
	if (entry_read_int(CHARACTER_TABLE.c_str(), getId().c_str(), &pos_ty,
			CHARACTER_KEY_TILE_Y, nullptr) == false)
	{
		ret = false;
	}

	setTile(pos_tx, pos_ty);

	context_unlock_list();
	return ret;
}

/*****************************************************************************/
Connection* Context::getConnection() const
{
	return m_connection;
}

/*****************************************************************************/
void Context::setConnection(Connection *connection)
{
	m_connection = connection;
}

/*****************************************************************************/
SDL_Thread* Context::getNpcThread() const
{
	return m_npcThread;
}

/*****************************************************************************/
void Context::setNpcThread(SDL_Thread *npcThread)
{
	m_npcThread = npcThread;
}

/*****************************************************************************/
void Context::addRunningAction(const std::string &action,
		RunningAction *runningAction)
{
	m_actionRunning.insert(
			std::pair<std::string, RunningAction*>(action, runningAction));
}

/*****************************************************************************/
void Context::stopRunningAction(const std::string &action)
{
	auto it = m_actionRunning.find(action);
	if (it != m_actionRunning.end())
	{
		it->second->stop();
		m_actionRunning.erase(it);
	}
}

/*****************************************************************************/
Lock& Context::getLuaVmLock()
{
	return m_luaVmLock;
}
