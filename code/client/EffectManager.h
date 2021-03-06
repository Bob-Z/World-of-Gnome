/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2017-2020 carabobz@gmail.com

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

#ifndef CLIENT_EFFECTMANAGER_H_
#define CLIENT_EFFECTMANAGER_H_

#include "Connection.h"
#include <string>
#include <vector>

class EffectManager
{
public:
	enum class EffectType
	{
		CONTEXT, MAP
	};

	static void processEffectFrame(Connection & connection, const std::vector<std::string> & params);
};

#endif // CLIENT_EFFECTMANAGER_H_
