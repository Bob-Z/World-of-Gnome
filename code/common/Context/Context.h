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

#ifndef CONTEXT_H
#define CONTEXT_H

#include "Connection.h"
#include "RunningAction.h"
#include "Selection.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

extern "C"
{
#include <lua.h>
}

class Context
{
public:
	Context();
	~Context();
	bool isInGame() const;
	void setInGame(bool inGame);
	bool isNpc() const;
	void setNpc(bool npc);
	bool isNpcActive() const;
	const std::string& getCharacterName() const;
	void setCharacterName(const std::string& characterName);
	const std::string& getMap() const;
	void setMap(const std::string& map);
	const std::string& getPreviousMap() const;
	void setPreviousMap(const std::string& previousMap);
	bool isMapChanged() const;
	void setMapChanged(bool mapChanged);
	int getTileX() const;
	int getTileY() const;
	void setTile(const int tileX, const int tileY);
	bool isPositionChanged() const;
	void setPositionChanged(bool positionChanged);
	int getDirection() const;
	void setDirection(int direction);
	int getOrientation() const;
	void setOrientation(int orientation);
	int getPreviousTileX() const;
	int getPreviousTileY() const;
	Uint32 getAnimationTick() const;
	void setAnimationTick(Uint32 animationTick);
	const std::string& getType() const;
	void setType(const std::string& type);
	const std::string& getId() const;
	void setId(const std::string& id);
	const Selection& getSelection() const;
	void setSelection(const Selection& selection);
	void setSelectionContextId(const std::string & contextId);
	const std::string& getSelectionContextId() const;
	void setSelectionTile(const std::string & map, int tx, int ty);
	const std::string& getSelectionMap() const;
	int getSelectionMapTx() const;
	int getSelectionMapTy() const;
	void setSelectionEquipment(const std::string & equipment);
	const std::string & getSelectionEquipment() const;
	void setSelectionInventory(const std::string & item);
	const std::string & getSelectionInventory() const;
	Uint32 getNextExecutionTick() const;
	void setNextExecutionTick(Uint32 nextExecutionTick);

	Connection* getConnection() const;
	void setConnection(Connection* connection);

	SDL_Thread* getNpcThread() const;
	void setNpcThread(SDL_Thread* npcThread);

	bool update_from_file();

	int tileDistance(const Context & ctx) const;

	lua_State* getLuaVm();
	void wakeUp();
	void sleep(Uint32 timeOutMs);

	void addRunningAction(const std::string & action, RunningAction * runningAction);
	void stopRunningAction(const std::string & action);

	SDL_mutex* getLuaVmMutex() const;

private:
	mutable SDL_mutex* m_mutex;
	bool m_inGame;
	bool m_npc;
	std::string m_characterName;
	std::string m_map;
	std::string m_previousMap;
	bool m_mapChanged; // FIXME
	int m_tileX;	// player position (in tile)
	int m_tileY;	// player position (in tile)
	int m_previousTileX;	// player previous position (in tile) for sprite direction
	int m_previousTileY;	// player previous position (in tile) for sprite direction
	bool m_positionChanged;
	int m_orientation;	// Bit field for sprite orientation (north east, south...)
	int m_direction;	// Bit field for sprite direction (north, south...)
	Uint32 m_animationTick;	// Start tick for animation
	std::string m_type;	// character's type
	std::string m_id; // unique ID of a character (its filename)
	Selection m_selection; // Selected tile or sprite
	Uint32 m_nextExecutionTick; // Time when an NPC will execute its AI script

	lua_State* m_luaVm;	// LUA state
	SDL_mutex * m_luaVmMutex;
	SDL_cond* m_condition;	// async condition for NPC
	SDL_mutex* m_conditionMutex;	// mutex for async condition for NPC

	Connection * m_connection;

	SDL_Thread * m_npcThread;

	std::map<std::string, RunningAction *> m_actionRunning;

public:
	Context * m_previous;
	Context * m_next;
};

Context * context_get_list_start();
Context * context_new(void);
void context_free(Context * context);
void context_set_socket(Context * context, TCPsocket socket);
TCPsocket context_get_socket(Context * context);
void context_set_socket_data(Context * context, TCPsocket socket);
TCPsocket context_get_socket_data(Context * context);

void context_spread(Context * context);
void context_add_or_update_from_network_frame(const Context & context);
void context_lock_list();
void context_unlock_list();
Context * context_get_first();
Context * context_get_player();
int context_write_to_file(Context * context);
Context * context_find(const std::string & id);

#endif
