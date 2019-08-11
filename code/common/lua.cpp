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

#include "common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include <lualib.h>
#include <lauxlib.h>

#ifdef __cplusplus
}
#endif

/*******************************************************************************
 Execute the given script with its parameters
 return -1 on error or return value from execution
 *******************************************************************************/
int lua_execute_script(lua_State* p_pLuaVm, const char * p_pScript,
		const char ** p_pParameters)
{
	// Load script
	char * l_pFullPath = nullptr;
	l_pFullPath = strconcat(base_directory, "/", SCRIPT_TABLE, "/", p_pScript,
			nullptr);

	if (luaL_loadfile(p_pLuaVm, l_pFullPath) != 0)
	{
		// If something went wrong, error message is at the top of the stack
		werr(LOGUSER, "Couldn't load LUA script %s: %s\n", l_pFullPath,
				lua_tostring(p_pLuaVm, -1));
		free(l_pFullPath);
		return -1;
	}

	// Fake call to read global variable from the script file (i.e. the f function
	lua_pcall(p_pLuaVm, 0, 0, 0);

	// push f function on LUA VM stack
	lua_getglobal(p_pLuaVm, "f");

	// push parameters on LUA VM stack (only strings parameters are supported)
	int l_ParamNum = 0;
	if (p_pParameters != nullptr)
	{
		while (p_pParameters[l_ParamNum] != nullptr)
		{
			lua_pushstring(p_pLuaVm, p_pParameters[l_ParamNum]);
			l_ParamNum++;
		}
	}

	// Ask LUA to call the f function with the given parameters
	// number of argument = l_ParamNum, number of result = 1
	if (lua_pcall(p_pLuaVm, l_ParamNum, 1, 0) != 0)
	{
		werr(LOGUSER, "Error running LUA script %s: %s\n", l_pFullPath,
				lua_tostring(p_pLuaVm, -1));
		free(l_pFullPath);
		return -1;
	}
	free(l_pFullPath);

	// we expect a number as the result
	if (!lua_isnumber(p_pLuaVm, -1))
	{
		lua_pop(p_pLuaVm, 1);
		return -1;
	}

	int l_ReturnValue;
	l_ReturnValue = lua_tonumber(p_pLuaVm, -1);

	// Remove returned value from stack
	lua_pop(p_pLuaVm, 1);

	return l_ReturnValue;
}

/*******************************************************************************
 Execute the given script with its parameters
 return -1 on error or return value from execution
 *******************************************************************************/
int lua_execute_script(lua_State* p_pLuaVm, const std::string & p_rScript,
		const std::vector<std::string> & p_rParams)
{
	// Load script
	char * l_pFullPath = nullptr;
	l_pFullPath = strconcat(base_directory, "/", SCRIPT_TABLE, "/",
			p_rScript.c_str(), nullptr);

	if (luaL_loadfile(p_pLuaVm, l_pFullPath) != 0)
	{
		// If something went wrong, error message is at the top of the stack
		werr(LOGUSER, "Couldn't load LUA script %s: %s\n", l_pFullPath,
				lua_tostring(p_pLuaVm, -1));
		free(l_pFullPath);
		return -1;
	}

	// Fake call to read global variable from the script file (i.e. the f function
	lua_pcall(p_pLuaVm, 0, 0, 0);

	// push f function on LUA VM stack
	lua_getglobal(p_pLuaVm, "f");

	// push parameters on LUA VM stack (only strings parameters are supported)
	int l_ParamNum = 0;
	for (auto l_It = p_rParams.begin(); l_It != p_rParams.end(); ++l_It)
	{
		lua_pushstring(p_pLuaVm, l_It->c_str());
		l_ParamNum++;
	}

	// Ask LUA to call the f function with the given parameters
	// number of argument = l_ParamNum, number of result = 1
	if (lua_pcall(p_pLuaVm, l_ParamNum, 1, 0) != 0)
	{
		werr(LOGUSER, "Error running LUA script %s: %s\n", l_pFullPath,
				lua_tostring(p_pLuaVm, -1));
		free(l_pFullPath);
		return -1;
	}
	free(l_pFullPath);

	// we expect a number as the result
	if (!lua_isnumber(p_pLuaVm, -1))
	{
		lua_pop(p_pLuaVm, 1);
		return -1;
	}

	int l_ReturnValue;
	l_ReturnValue = lua_tonumber(p_pLuaVm, -1);

	// Remove returned value from stack
	lua_pop(p_pLuaVm, 1);

	return l_ReturnValue;
}
