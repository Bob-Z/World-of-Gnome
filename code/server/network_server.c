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
Broadcast text to all connected players
*********************************************************************/
void network_broadcast_text(context_t * context, const char * text)
{
	context_t * ctx = NULL;

	SDL_LockMutex(context_list_mutex);

	ctx = context_get_list_first();

	if( ctx == NULL ) {
		SDL_UnlockMutex(context_list_mutex);
		return;
	}

	do {
		/* Skip if not connected (NPC) */
		if( context_get_socket(ctx) == 0 ) {
			continue;
		}

		/* Skip data transmission network */
		if( ctx->user_name == NULL ) {
			continue;
		}

		/* Skip if not on the same map */
#if 0
		if( same_map_only ) {
			if( target == NULL ) {
				continue;
			}
			if( strcmp(target->map,ctx->map) != 0 ) {
				continue;
			}
		}
#endif
		network_send_text(ctx->id,text);
	} while( (ctx=ctx->next)!= NULL );

	SDL_UnlockMutex(context_list_mutex);
}

/*********************************************************************
Server send the full character's file to client
*********************************************************************/
void network_send_character_file(context_t * context)
{
	char * filename;

	filename = strconcat(CHARACTER_TABLE,"/",context->id,NULL);
	network_send_file(context,filename);
	free(filename);
}

/*********************************************************************
Asks to update an int entry on all connected contexts
*********************************************************************/
void network_broadcast_entry_int(const char * table, const char * file, const char * path, int value, int same_map_only)
{
	context_t * ctx = NULL;
	context_t * target = NULL;

	target = context_find(file);

	SDL_LockMutex(context_list_mutex);

	ctx = context_get_list_first();

	if( ctx == NULL ) {
		SDL_UnlockMutex(context_list_mutex);
		return;
	}

	do {
		/* Skip if not connected (NPC) */
		if( context_get_socket(ctx) == 0 ) {
			continue;
		}
		/* Skip if not on the same map */
		if( same_map_only ) {
			if( target == NULL ) {
				continue;
			}
			if( target->map && ctx->map ) {
				if( strcmp(target->map,ctx->map) != 0 ) {
					continue;
				}
			}
		}

		network_send_entry_int(ctx,table,file, path, value);

	} while( (ctx=ctx->next)!= NULL );

	SDL_UnlockMutex(context_list_mutex);
}

/*********************************************************************
*********************************************************************/
static int new_connection(void * data)
{
	context_t * context;
	TCPsocket socket = (TCPsocket)data;
	Uint32 command = 0;
	Uint32 command_size = 0;
	char *buf = NULL;
	
	context = context_new();
	if(context == NULL ) {
		werr(LOGUSER,"Failed to create context");
		return TRUE;
	}

	context_set_socket(context,socket);

	context_new_VM(context);

	context_set_connected(context,TRUE);

	while(context_get_connected(context)) {
		/* Read a command code */
		if( !network_read_bytes(socket,(char *)&command, sizeof(Uint32))) {
			context_set_connected(context,FALSE);
			break;
		}
		/* Read a size */
		if( !network_read_bytes(socket,(char *)&command_size, sizeof(Uint32))) {
			context_set_connected(context,FALSE);
			break;
		}

		/* Read additional data */
		if( command_size > 0) {
			buf = malloc(command_size);
			if( !network_read_bytes(socket,buf, command_size)) {
				context_set_connected(context,FALSE);
				break;
			}
		}

		if (!parse_incoming_data(context, command, command_size, buf) ) {
			if( buf ) {
				free(buf);
				buf = NULL;
			}
			context_set_connected(context,FALSE);
			break;
		}

		if( buf != NULL) {
			free(buf);
			buf = NULL;
		}
	}
	
	SDLNet_TCP_Close(socket);
	wlog(LOGUSER,"Client disconnected");
	context_spread(context);
	context_write_to_file(context);
	context_free(context);

	return TRUE;
}

/*********************************************************************
*********************************************************************/
void network_init(void)
{
	IPaddress IP;
	IPaddress * remote_IP;
	TCPsocket socket;
	TCPsocket client_socket;
	
	if (SDLNet_Init() < 0)
	{
		werr(LOGUSER, "Can't init SDL: %s\n", SDLNet_GetError());
		return;
	}
 
	/* Resolving the host using NULL make network interface to listen */
	if (SDLNet_ResolveHost(&IP, NULL, PORT) < 0)
	{
		werr(LOGUSER, "Cannot listen on port %d: %s\n", PORT, SDLNet_GetError());
		return;
	}
 
	/* Open a connection with the IP provided (listen on the host's port) */
	if (!(socket = SDLNet_TCP_Open(&IP)))
	{
		werr(LOGUSER, "Cannot open port %d: %s\n", PORT, SDLNet_GetError());
		return;
	}
 
	/* Wait for a connection */
	while (TRUE)
	{
		/* check for pending connection.
		* If there is one, accept that, and open a new socket for communicating */
		if ((client_socket = SDLNet_TCP_Accept(socket)))
		{
			/* Get the remote address */
			if (!(remote_IP = SDLNet_TCP_GetPeerAddress(client_socket))) {
				werr(LOGUSER,"Can't get peer adress: %s", SDLNet_GetError());
			}

			//wlog(LOGUSER,"Host connected: %s %d\n", SDLNet_Read32(&remote_IP->host), SDLNet_Read16(&remote_IP->port));
			wlog(LOGUSER,"Host connected: %x %d\n", SDLNet_Read32(&remote_IP->host), SDLNet_Read16(&remote_IP->port));
 
			SDL_CreateThread(new_connection,"new_connection",(void*)client_socket);
		}
	}
 
	SDLNet_TCP_Close(socket);
	SDLNet_Quit();
		
	return;
}
