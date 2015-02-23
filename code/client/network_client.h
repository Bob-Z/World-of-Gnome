/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013-2015 carabobz@gmail.com

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

#include "../common/common.h"

void network_login(context_t * context, const char * name, const char * password);
void network_request_start(context_t * context, const char * id);
void network_request_stop(context_t * context);
void network_login(context_t * context, const char * name, const char * password);
void network_request_character_list(context_t * context);
void network_request_user_character_list(context_t * context);
void network_send_action(context_t * context, char * frame,...);
int network_connect(context_t * context, const char * hostname);
int network_open_data_connection(context_t * context);

#endif
