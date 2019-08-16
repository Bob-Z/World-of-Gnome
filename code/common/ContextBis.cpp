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

#include "ContextBis.h"

/******************************************************************************/
ContextBis::ContextBis() :
		m_user_name(), m_connected(false), m_in_game(false), m_npc(false), m_character_name(), m_map(), m_tile_x(0), m_tile_y(0), m_type(), m_selection(), m_id()
{
}

/******************************************************************************/
ContextBis::~ContextBis()
{
}

/******************************************************************************/
const std::string& ContextBis::getCharacterName() const
{
	return m_character_name;
}

/******************************************************************************/
void ContextBis::setCharacterName(const std::string& characterName)
{
	m_character_name = characterName;
}

/******************************************************************************/
bool ContextBis::isConnected() const
{
	return m_connected;
}

/******************************************************************************/
void ContextBis::setConnected(bool connected)
{
	m_connected = connected;
}

/******************************************************************************/
const std::string& ContextBis::getId() const
{
	return m_id;
}

/******************************************************************************/
void ContextBis::setId(const std::string& id)
{
	m_id = id;
}

/******************************************************************************/
bool ContextBis::isInGame() const
{
	return m_in_game;
}

/******************************************************************************/
void ContextBis::setInGame(bool inGame)
{
	m_in_game = inGame;
}

/******************************************************************************/
const std::string& ContextBis::getMap() const
{
	return m_map;
}

/******************************************************************************/
void ContextBis::setMap(const std::string& map)
{
	m_map = map;
}

/******************************************************************************/
bool ContextBis::isNpc() const
{
	return m_npc;
}

/******************************************************************************/
void ContextBis::setNpc(bool npc)
{
	m_npc = npc;
}

/******************************************************************************/
const Selection& ContextBis::getSelection() const
{
	return m_selection;
}

/******************************************************************************/
void ContextBis::setSelection(const Selection& selection)
{
	m_selection = selection;
}

/******************************************************************************/
int ContextBis::getTileX() const
{
	return m_tile_x;
}

/******************************************************************************/
void ContextBis::setTileX(int tileX)
{
	m_tile_x = tileX;
}

/******************************************************************************/
int ContextBis::getTileY() const
{
	return m_tile_y;
}

/******************************************************************************/
void ContextBis::setTileY(int tileY)
{
	m_tile_y = tileY;
}

/******************************************************************************/
const std::string& ContextBis::getType() const
{
	return m_type;
}

/******************************************************************************/
void ContextBis::setType(const std::string& type)
{
	m_type = type;
}

/******************************************************************************/
const std::string& ContextBis::getUserName() const
{
	return m_user_name;
}

/******************************************************************************/
void ContextBis::setUserName(const std::string& userName)
{
	m_user_name = userName;
}
