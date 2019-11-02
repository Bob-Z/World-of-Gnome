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

#ifndef NETWORK_H
#define NETWORK_H

#include "Context.h"
#include <SDL_net.h>
#include <string>

int network_read_bytes(TCPsocket socket, char * data, int size);
void network_send_command(Context * p_pContext, const std::string & serialized_data, const bool p_IsData);
int network_send_file(Context * context, const char * filename);
void network_send_file_data(Context * context, const std::string & name, const std::string & data);
int network_send_table_file(Context * context, const char * table, const char * filename);
void network_send_req_file(Context * context, const std::string & file_name);

// The code of this function is in parser_client.c and parser_server.c
int parse_incoming_data(Context * context, const std::string & serialized_data);
#endif
