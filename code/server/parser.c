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

#include <glib.h>
#include <gio/gio.h>
#include "../common/common.h"
#include "avatar.h"
#include "action.h"
#include <string.h>

/* parse_incoming_data

Return FALSE on error, TRUE if OK */
gboolean parse_incoming_data(context_t * context, guint32 command, guint32 command_size, gchar * data)
{
	const gchar * value = NULL;
	gchar * filename;
	gchar ** elements;

	//g_message("Received command : %d, command_size : %d", command, command_size);

	if( !context_get_connected(context) && command != CMD_LOGIN_USER && command != CMD_LOGIN_PASSWORD ) {
		g_warning("Request from not authenticated client, close connection");
		return FALSE;
	}

	switch(command) {
		case CMD_LOGIN_USER :
			g_message("Received CMD_LOGIN_USER");
			if( !context_set_username(context, data) ) {
				return FALSE;
			}
			g_message("Successfully set username to %s",data);
			break;
		case CMD_LOGIN_PASSWORD :
			g_message("Received CMD_LOGIN_PASSWORD");
			/* Read username / password pairs from file : FILE_USER_CONF */
			/* Read password for username */
			if(!read_string(PASSWD_TABLE, context->user_name, &value, PASSWD_KEY_PASSWORD,NULL)) {
				return FALSE;
			}
			if( g_strcmp0(value, data) != 0) {
				g_message("Wrong login for %s",context->user_name);
				/* send answer */
				network_send_command(context, CMD_LOGIN_NOK, 0, NULL, FALSE);
				/* force client disconnection*/
				return FALSE;
			}
			else {
				/* send answer */
				network_send_command(context, CMD_LOGIN_OK, 0, NULL, FALSE);
				g_message("%s successfully login",context->user_name);
				context_set_connected(context, TRUE);
			}
			break;
		case CMD_REQ_AVATAR_LIST :
			g_message("Received CMD_REQ_AVATAR_LIST");
			avatar_send_list(context);
			g_message("avatar list sent");
			break;
		case CMD_REQ_FILE :
			elements = g_strsplit(data,NETWORK_DELIMITER,0);
			g_message("Received CMD_REQ_FILE for %s",elements[0]);
			/* compare checksum */
			filename = g_strconcat( g_getenv("HOME"),"/", base_directory, "/", elements[0], NULL);
			gchar * cksum = checksum_file(filename);
			g_free(filename);
			if( cksum == NULL) {
				g_warning("No file named %s",elements[0]);
				g_strfreev(elements);
				break;
			}

			if( g_strcmp0(elements[1],cksum) == 0 ) {
				g_message("Client has already newest %s file",elements[0]);
				g_strfreev(elements);
				break;
			}
			g_free(cksum);
			network_send_file(context,elements[0]);
			g_message("File %s sent",elements[0]);
			g_strfreev(elements);
			break;
		case CMD_REQ_USER_AVATAR_LIST :
			g_message("Received CMD_REQ_USER_AVATAR_LIST");
			avatar_user_send_list(context);
			g_message("user %s's avatars list sent",context->user_name);
			break;
		case CMD_SEND_CONTEXT :
			if( context->type == NULL ) { /* First time a context send its data */
				context_update_from_network_frame(context,data);
				context_spread(context);
				context_request_other_context(context);
			}
			else {
				context_update_from_network_frame(context,data);
			}
			context_write_to_file(context);
			g_message("Received CMD_SEND_CONTEXT for %s /%s",context->user_name,context->avatar_name);
			break;
		case CMD_SEND_ACTION :
			g_debug("Received CMD_SEND_ACTION");
			elements = g_strsplit(data,NETWORK_DELIMITER,0);
			action_execute_script(context,elements[0],&elements[1]);
			g_strfreev(elements);
			break;
		default:
			g_warning("Unknown request %d from client",command);
			return FALSE;
	}

	return TRUE;
}

