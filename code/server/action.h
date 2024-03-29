/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2020 carabobz@gmail.com

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

#ifndef ACTION_SERVER_H
#define ACTION_SERVER_H

#include <string>
#include <vector>

class Context;

void action_parse_frame(Context *context, char *frame);
void action_run_or_execute(Context *context, const std::string &actionName,
		const std::vector<std::string> &parameters);
int action_execute(Context *context, const std::string &actionName,
		const std::vector<std::string> &parameters);
void action_run(Context *context, const std::string &actionName,
		const std::vector<std::string> &parameters);
void action_stop(Context *context, const std::string &actionName);
int action_execute_script(Context *context, const std::string &scriptName,
		const std::vector<std::string> &params);
void register_lua_functions(Context *context);

#endif
