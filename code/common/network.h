/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013-2014 carabobz@gmail.com

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

#include "context.h"

typedef struct speak_entry {
	char * icon;
	char * text;
	char * keyword;
} speak_entry_t;

int network_read_bytes(TCPsocket socket, char * data, int size);
void network_send_command(context_t * context, Uint32 command, long int count, const char *data, int is_data);
void network_send_context(context_t * context);
void network_send_context_to_context(context_t * dest_ctx, context_t * src_ctx);
int network_send_file(context_t * context, char * filename);
int network_send_table_file(context_t * context, char * table, char * filename);
void network_send_req_file(context_t * context, char * file);
void network_send_entry_int(context_t * context, const char * table, const char * file, const char *path, int value);
void network_send_text(const char * id, const char * string);

/* The code of this function is in parser_client.c and parser_server.c */
int parse_incoming_data(context_t * context, Uint32 command, Uint32 command_size, char * data);
#endif
