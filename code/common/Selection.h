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

#ifndef COMMON_SELECTION_H
#define COMMON_SELECTION_H

#include <string>

class Selection
{
public:
	Selection();
	virtual ~Selection();

	std::string getEquipment() const;
	void setEquipment(std::string equipment);
	std::string getId() const;
	void setId(std::string id);
	std::string getInventory() const;
	void setInventory(std::string inventory);
	std::string getMap() const;
	void setMap(std::string map);
	int getMapCoordTx() const;
	void setMapCoordTx(int mapCoordTx);
	int getMapCoordTy() const;
	void setMapCoordTy(int mapCoordTy);

private:
	std::string m_id;
	int m_map_coord_tx;
	int m_map_coord_ty;
	std::string m_map;
	std::string m_inventory;
	std::string m_equipment;
};

#endif // COMMON_SELECTION_H_
