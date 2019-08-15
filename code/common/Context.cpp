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

/******************************************************************************/
Context::Context() :
		m_user_name(), m_connected(false), m_in_game(false), m_npc(false), m_character_name(), m_map(), m_tile_x(
				0), m_tile_y(0), m_type(), m_selection(), m_id()
{
}

/******************************************************************************/
Context::~Context()
{
}

/******************************************************************************/
const std::string& Context::getCharacterName() const
{
	return m_character_name;
}

/******************************************************************************/
void Context::setCharacterName(const std::string& characterName)
{
	m_character_name = characterName;
}

/******************************************************************************/
bool Context::isConnected() const
{
	return m_connected;
}

/******************************************************************************/
void Context::setConnected(bool connected)
{
	m_connected = connected;
}

/******************************************************************************/
const std::string& Context::getId() const
{
	return m_id;
}

/******************************************************************************/
void Context::setId(const std::string& id)
{
	m_id = id;
}

/******************************************************************************/
bool Context::isInGame() const
{
	return m_in_game;
}

/******************************************************************************/
void Context::setInGame(bool inGame)
{
	m_in_game = inGame;
}

/******************************************************************************/
const std::string& Context::getMap() const
{
	return m_map;
}

/******************************************************************************/
void Context::setMap(const std::string& map)
{
	m_map = map;
}

/******************************************************************************/
bool Context::isNpc() const
{
	return m_npc;
}

/******************************************************************************/
void Context::setNpc(bool npc)
{
	m_npc = npc;
}

/******************************************************************************/
const Selection& Context::getSelection() const
{
	return m_selection;
}

/******************************************************************************/
void Context::setSelection(const Selection& selection)
{
	m_selection = selection;
}

/******************************************************************************/
int Context::getTileX() const
{
	return m_tile_x;
}

/******************************************************************************/
void Context::setTileX(int tileX)
{
	m_tile_x = tileX;
}

/******************************************************************************/
int Context::getTileY() const
{
	return m_tile_y;
}

/******************************************************************************/
void Context::setTileY(int tileY)
{
	m_tile_y = tileY;
}

/******************************************************************************/
const std::string& Context::getType() const
{
	return m_type;
}

/******************************************************************************/
void Context::setType(const std::string& type)
{
	m_type = type;
}

/******************************************************************************/
const std::string& Context::getUserName() const
{
	return m_user_name;
}

/******************************************************************************/
void Context::setUserName(const std::string& userName)
{
	m_user_name = userName;
}
