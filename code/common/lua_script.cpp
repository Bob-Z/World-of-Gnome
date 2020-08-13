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

#include "LockGuard.h"
#include "client_server.h"
#include "log.h"
#include "lua_script.h"
#include "syntax.h"
#include <iterator>
#include <string>
#include <vector>

#ifdef __cplusplus
extern "C"
{
#endif
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#ifdef __cplusplus
}
#endif

/*******************************************************************************
 Execute the given script with its parameters
 return -1 on error or return value from execution
 *******************************************************************************/
int lua_execute_script(lua_State* lua_vm, Lock & lock, const char * script, const char ** parameters)
{
	LockGuard guard(lock);

	// Load script
	const std::string file_path = base_directory + "/" + std::string(SCRIPT_TABLE) + "/" + std::string(script);

	if (luaL_loadfile(lua_vm, file_path.c_str()) != 0)
	{
		// If something went wrong, error message is at the top of the stack
		werr(LOGUSER, "Couldn't load LUA script %s: %s\n", file_path.c_str(), lua_tostring(lua_vm, -1));
		return -1;
	}

	// Fake call to read global variable from the script file (i.e. the f function
	if (lua_pcall(lua_vm, 0, 0, 0) != 0)
	{
		werr(LOGDEVELOPER, "Error calling lua_pcall");
	}

	// push f function on LUA VM stack
	lua_getglobal(lua_vm, "f");

	// push parameters on LUA VM stack (only strings parameters are supported)
	int param_qty = 0;
	if (parameters != nullptr)
	{
		while (parameters[param_qty] != nullptr)
		{
			lua_pushstring(lua_vm, parameters[param_qty]);
			param_qty++;
		}
	}

	// Ask LUA to call the f function with the given parameters
	// number of argument = param_qty, number of result = 1
	if (lua_pcall(lua_vm, param_qty, 1, 0) != 0)
	{
		werr(LOGUSER, "Error running LUA script %s: %s\n", file_path.c_str(), lua_tostring(lua_vm, -1));
		return -1;
	}

	// we expect a number as the result
	if (lua_isnumber(lua_vm, -1) == 0)
	{
		lua_pop(lua_vm, 1);
		return -1;
	}

	int l_ReturnValue;
	l_ReturnValue = lua_tonumber(lua_vm, -1);

	// Remove returned value from stack
	lua_pop(lua_vm, 1);

	return l_ReturnValue;
}

/*******************************************************************************
 Execute the given script with its parameters
 return -1 on error or return value from execution
 *******************************************************************************/
int lua_execute_script(lua_State* lua_vm, Lock & lock, const std::string & script, const std::vector<std::string> & parameterArray)
{
	LockGuard guard(lock);

	// Load script
	const std::string file_path = base_directory + "/" + std::string(SCRIPT_TABLE) + "/" + script;

	if (luaL_loadfile(lua_vm, file_path.c_str()) != 0)
	{
		// If something went wrong, error message is at the top of the stack
		werr(LOGUSER, "Couldn't load LUA script %s: %s\n", file_path.c_str(), lua_tostring(lua_vm, -1));
		return -1;
	}

	// Fake call to read global variable from the script file (i.e. the f function
	lua_pcall(lua_vm, 0, 0, 0);

	// push f function on LUA VM stack
	lua_getglobal(lua_vm, "f");

	// push parameters on LUA VM stack (only strings parameters are supported)
	int paramQty = 0;
	for (auto && parameter : parameterArray)
	{
		lua_pushstring(lua_vm, parameter.c_str());
		paramQty++;
	}

	// Ask LUA to call the f function with the given parameters
	// number of argument = l_ParamNum, number of result = 1
	if (lua_pcall(lua_vm, paramQty, 1, 0) != 0)
	{
		werr(LOGUSER, "Error running LUA script %s: %s\n", file_path.c_str(), lua_tostring(lua_vm, -1));
		return -1;
	}

	// we expect a number as the result
	if (!lua_isnumber(lua_vm, -1))
	{
		lua_pop(lua_vm, 1);
		return -1;
	}

	int l_ReturnValue;
	l_ReturnValue = lua_tonumber(lua_vm, -1);

	// Remove returned value from stack
	lua_pop(lua_vm, 1);

	return l_ReturnValue;
}
