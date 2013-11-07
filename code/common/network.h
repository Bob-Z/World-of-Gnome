/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013 carabobz@gmail.com

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

#include <glib.h>
#include "common.h"

void network_init(void);
int network_open_data_connection(context_t * context);
void network_send_command(context_t * context, guint32 command, gsize count, const gchar *data,gboolean is_data);
void network_send_avatar_file(context_t * context);
gboolean network_connect(context_t * context, const gchar * hostname);
void network_login(context_t * context, const gchar * name, const gchar * password);
void network_request_avatar_list(context_t * context);
void network_request_avatar_marquee(context_t * context, gchar * avatar_name);
void network_request_user_avatar_list(context_t * context);
void network_request_avatar_data(context_t * context);
void network_send_context(context_t * context);
void network_send_context_to_context(context_t * dest_ctx, context_t * src_ctx);
void network_send_text(const gchar * id, const gchar * string);
void network_broadcast_text(context_t * context, const gchar * text);
void network_send_action(context_t * context, gchar * frame,...);
int network_send_file(context_t * context, gchar * filename);
void network_send_req_file(context_t * context, gchar * file);
void network_send_entry_int(context_t * context, const gchar * table, const gchar * file, const gchar *path, int value);
void network_broadcast_entry_int(const gchar * table, const gchar * file, const gchar * path, gint value, gboolean same_map_only);

/* The code of this function is in parser.c in both client and server directory */
gboolean parse_incoming_data(context_t * context, guint32 command, guint32 command_size, gchar * data);
#endif
