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

#include "../common/common.h"

/*********************************************************************
sends a login request, the answer is asynchronously read by async_recv
**********************************************************************/
void network_login(context_t * context, const char * user_name, const char * password)
{
	char * frame;

	frame = strconcat(user_name,NETWORK_DELIMITER,password,NULL);

	wlog(LOGDEBUG,"Send CMD_REQ_LOGIN");
	network_send_command(context, CMD_REQ_LOGIN, strlen(frame) + 1, frame,FALSE);
	free(frame);
}

/*********************************************************************
*********************************************************************/
void network_request_character_list(context_t * context)
{
	wlog(LOGDEBUG,"Send CMD_REQ_CHARACTER_LIST");
	network_send_command(context, CMD_REQ_CHARACTER_LIST, 0, NULL,FALSE);
}

/*********************************************************************
request a specific user's characters list
*********************************************************************/
void network_request_user_character_list(context_t * context)
{
	wlog(LOGDEBUG,"Send CMD_REQ_USER_CHARACTER_LIST");
	network_send_command(context, CMD_REQ_USER_CHARACTER_LIST, strlen(context->user_name)+1, context->user_name,FALSE);
}

/*********************************************************************
Player sends an action to server
*********************************************************************/
void network_send_action(context_t * context, char * script,...)
{
	va_list ap;
	char * frame;
	char * new_frame;
	char * parameter;

	if( script == NULL ) {
		return;
	}

	frame = strdup(script);
	va_start(ap, script);
	while ( (parameter=va_arg(ap,char*)) != NULL ) {
		new_frame = strconcat(frame,NETWORK_DELIMITER,parameter,NULL);
		free(frame);
		frame = new_frame;
	}
	va_end(ap);

	wlog(LOGDEBUG,"Send CMD_REQ_ACTION :%s",frame);
	network_send_command(context, CMD_REQ_ACTION, strlen(frame)+1, frame,FALSE);
	free(frame);
}

/*********************************************************************
Callback from client listening to server in its own thread
only used for game information transfer
*********************************************************************/
static int async_recv(void * data)
{
	context_t * context = (context_t *)data;

	Uint32 command = 0;
	Uint32 command_size = 0;
	char *buf = NULL;

	while(1) {
		command = 0;
		command_size = 0;
		buf = NULL;

		if( !network_read_bytes(context->socket,(char *)&command, sizeof(Uint32)) ) {
			break;
		}
		/* Read a size */
		if( !network_read_bytes(context->socket,(char *)&command_size, sizeof(Uint32))) {
			break;
		}

		/* Read additional data */
		if( command_size > 0) {
			buf = malloc(command_size);
			if( !network_read_bytes(context->socket,buf, command_size) ) {
				break;
			}
		}

		if (!parse_incoming_data(context, command, command_size, buf) ) {
			if( buf ) {
				free(buf);
				buf = NULL;
			}
			break;
		}

		if( buf != NULL) {
			free(buf);
			buf = NULL;
		}
	}

	werr(LOGUSER,"Socket closed on server side.");

	context_set_connected(context,FALSE);
	SDLNet_TCP_Close(context->socket);
	SDLNet_TCP_Close(context->socket_data);
	context_set_socket(context,0);
	context_set_socket_data(context,0);

	return 0;
}

/*********************************************************************
Callback from client listening to server in its own thread
Only used for data transfers
*********************************************************************/
static int async_data_recv(void * data)
{
	context_t * context = (context_t *)data;

	Uint32 command = 0;
	Uint32 command_size = 0;
	char *buf = NULL;

	while(1) {
		command = 0;
		command_size = 0;
		buf = NULL;

		if( !network_read_bytes(context->socket_data,(char *)&command, sizeof(Uint32))) {
			break;
		}
		/* Read a size */
		if( !network_read_bytes(context->socket_data,(char *)&command_size, sizeof(Uint32)) ) {
			break;
		}

		/* Read additional data */
		if( command_size > 0) {
			buf = malloc(command_size);
			if( !network_read_bytes(context->socket_data,buf, command_size) ) {
				break;
			}
		}

		if (!parse_incoming_data(context, command, command_size, buf) ) {
			if( buf ) {
				free(buf);
				buf = NULL;
			}
			break;
		}

		if( buf != NULL) {
			free(buf);
			buf = NULL;
		}
	}

	werr(LOGUSER,"Socket closed on server side.");

	context_set_connected(context,FALSE);
	SDLNet_TCP_Close(context->socket);
	SDLNet_TCP_Close(context->socket_data);
	context_set_socket(context,0);
	context_set_socket_data(context,0);

	return 0;
}

/*********************************************************************
return FALSE on error
return TRUE if OK
*********************************************************************/
int network_connect(context_t * context, const char * hostname)
{
	IPaddress ip;
	TCPsocket socket;

	wlog(LOGUSER, "Trying to connect to %s:%d", hostname, PORT);

	if (SDLNet_Init() < 0) {
		werr(LOGUSER, "Can't init SDLNet: %s\n", SDLNet_GetError());
		return FALSE;
	}

	if (SDLNet_ResolveHost(&ip, hostname, PORT) < 0) {
		werr(LOGUSER, "Can't resolve %s:%d : %s\n", hostname,PORT,SDLNet_GetError());
		return FALSE;
	}

	if (!(socket = SDLNet_TCP_Open(&ip))) {
		werr(LOGUSER, "Can't connect to %s:%d : %s\n", hostname,PORT,SDLNet_GetError());
		return FALSE;
	}

	wlog(LOGUSER,"Connected to %s:%d",hostname,PORT);

	context_set_hostname(context,hostname);
	context_set_socket(context, socket);

	SDL_CreateThread(async_recv,"async_recv",(void*)context);

	return TRUE;
}

/*********************************************************************
*********************************************************************/
int network_open_data_connection(context_t * context)
{
	IPaddress ip;
	TCPsocket socket;

	if (SDLNet_ResolveHost(&ip, context->hostname, PORT) < 0) {
		werr(LOGUSER, "Can't resolve %s:%d : %s\n", context->hostname,PORT,SDLNet_GetError());
		return FALSE;
	}

	if (!(socket = SDLNet_TCP_Open(&ip))) {
		werr(LOGUSER, "Can't open data connection to %s:%d : %s\n", context->hostname,PORT,SDLNet_GetError());
		return FALSE;
	}

	context_set_socket_data(context, socket);

	SDL_CreateThread(async_data_recv,"async_data_recv",(void*)context);

	return TRUE;
}
