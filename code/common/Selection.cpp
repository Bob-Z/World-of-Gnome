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
		m_id(), m_map_coord_tx(0), m_map_coord_ty(0), m_map(), m_inventory(), m_equipment()
{
}

/******************************************************************************/
Selection::~Selection()
{
}

/******************************************************************************/
std::string Selection::getEquipment() const
{
	return m_equipment;
}

/******************************************************************************/
void Selection::setEquipment(std::string equipment)
{
	m_equipment = equipment;
}

/******************************************************************************/
std::string Selection::getId() const
{
	return m_id;
}

/******************************************************************************/
void Selection::setId(std::string id)
{
	m_id = id;
}

/******************************************************************************/
std::string Selection::getInventory() const
{
	return m_inventory;
}

/******************************************************************************/
void Selection::setInventory(std::string inventory)
{
	m_inventory = inventory;
}

/******************************************************************************/
std::string Selection::getMap() const
{
	return m_map;
}

/******************************************************************************/
void Selection::setMap(std::string map)
{
	m_map = map;
}

/******************************************************************************/
int Selection::getMapCoordTx() const
{
	return m_map_coord_tx;
}

/******************************************************************************/
void Selection::setMapCoordTx(int mapCoordTx)
{
	m_map_coord_tx = mapCoordTx;
}

/******************************************************************************/
int Selection::getMapCoordTy() const
{
	return m_map_coord_ty;
}

/******************************************************************************/
void Selection::setMapCoordTy(int mapCoordTy)
{
	m_map_coord_ty = mapCoordTy;
}
