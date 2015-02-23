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

#ifndef NETWORK_SERVER
#define NETWORK_SERVER

#include "../common/common.h"

void network_broadcast_text(context_t * context, const char * text);
void network_send_character_file(context_t * context);
void network_broadcast_entry_int(const char * table, const char * file, const char * path, int value, int same_map_only);
void network_init(void);
void network_send_popup(const char * id, const char ** dialog);


#endif
