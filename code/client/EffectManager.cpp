/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2017-2019 carabobz@gmail.com

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

#include "EffectManager.h"
#include "file.h"
#include "lua_client.h"

/******************************************************************************/
void EffectManager::processEffectFrame(Context * context,
		const std::vector<std::string> & params)
{
	std::string script = params.front();
	const std::vector<std::string> script_params(params.begin() + 1,
			params.end());

	// TODO use the same LUA VM as the one in render screen
	if (lua_execute_script(getEffectLuaVm(), script, script_params) == -1)
	{
		file_request_from_network(context, SCRIPT_TABLE, script.c_str());
	}
}
