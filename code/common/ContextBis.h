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

#ifndef COMMON_ContextBisBIS_H_
#define COMMON_ContextBisBIS_H_

#include "Selection.h"
#include <string>

class ContextBis
{
public:
	ContextBis();
	virtual ~ContextBis();

	const std::string& getCharacterName() const;
	void setCharacterName(const std::string& characterName);
	bool isConnected() const;
	void setConnected(bool connected);
	const std::string& getId() const;
	void setId(const std::string& id);
	bool isInGame() const;
	void setInGame(bool inGame);
	const std::string& getMap() const;
	void setMap(const std::string& map);
	bool isNpc() const;
	void setNpc(bool npc);
	const Selection& getSelection() const;
	void setSelection(const Selection& selection);
	int getTileX() const;
	void setTileX(int tileX);
	int getTileY() const;
	void setTileY(int tileY);
	const std::string& getType() const;
	void setType(const std::string& type);
	const std::string& getUserName() const;
	void setUserName(const std::string& userName);

private:
	std::string m_user_name;
	bool m_connected; // User logged with the correct password, or NPC activated
	bool m_in_game;
	bool m_npc;
	std::string m_character_name;
	std::string m_map;	// map name
	int m_tile_x;	// player position (in tile)
	int m_tile_y;	// player position (in tile)
	std::string m_type;	// character's type
	Selection m_selection; // Selected tile or sprite
	std::string m_id; // unique ID of a character (its filename)
};

#endif // COMMON_ContextBisBIS_H_
