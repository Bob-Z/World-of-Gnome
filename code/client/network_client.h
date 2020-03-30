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

#ifndef NETWORK_CLIENT
#define NETWORK_CLIENT

#include <string>

class Context;

void network_login(Connection & connection, const std::string & name, const std::string & password);
void network_request_start(Connection & connection, const std::string & id);
void network_request_stop(Connection & connection);
void network_request_character_creation(Connection & connection, const char * id, const char * name);
void network_login(Context * context, const char * name, const char * password);
void network_request_playable_character_list(Connection & connection);
void network_request_user_character_list(Connection & connection);
void network_send_action(Connection & connection, const char * actionFile, ...);
void network_send_action_stop(Connection & connection, const char * actionFile);
int network_connect(Connection & connection, const std::string & host_name);
int network_open_data_connection(Connection & connection);

#endif
