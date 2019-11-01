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

#include "Selection.h"
#include <string>

/******************************************************************************/
Selection::Selection() :
		m_contextId(), m_mapTx(-1), m_mapTy(-1), m_map(), m_inventory(), m_equipment()
{
}

/******************************************************************************/
Selection::~Selection()
{
}

/******************************************************************************/
const std::string & Selection::getEquipment() const
{
	return m_equipment;
}

/******************************************************************************/
void Selection::setEquipment(std::string equipment)
{
	m_equipment = equipment;
}

/******************************************************************************/
const std::string & Selection::getContextId() const
{
	return m_contextId;
}

/******************************************************************************/
void Selection::setContextId(std::string id)
{
	m_contextId = id;
}

/******************************************************************************/
const std::string & Selection::getInventory() const
{
	return m_inventory;
}

/******************************************************************************/
void Selection::setInventory(std::string inventory)
{
	m_inventory = inventory;
}

/******************************************************************************/
const std::string & Selection::getMap() const
{
	return m_map;
}

/******************************************************************************/
void Selection::setMap(std::string map)
{
	m_map = map;
}

/******************************************************************************/
int Selection::getMapTx() const
{
	return m_mapTx;
}

/******************************************************************************/
void Selection::setMapTx(int mapCoordTx)
{
	m_mapTx = mapCoordTx;
}

/******************************************************************************/
int Selection::getMapTy() const
{
	return m_mapTy;
}

/******************************************************************************/
void Selection::setMapTy(int mapCoordTy)
{
	m_mapTy = mapCoordTy;
}
