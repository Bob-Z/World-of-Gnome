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

#include "common.h"
#include "NetworkFrame.h"
#include "wog.pb.h"
#include <arpa/inet.h>
#include <string>
#include <vector>

/*******************************************************************************
 Broadcast text to all in game players
 ******************************************************************************/
void network_broadcast_text(context_t * context, const char * text)
{
	context_t * ctx = nullptr;

	context_lock_list();

	ctx = context_get_first();

	if (ctx == nullptr)
	{
		context_unlock_list();
		return;
	}

	do
	{
		if (context_is_npc(ctx) == true)
		{
			continue;
		}

		// Skip if not in game
		if (context_get_in_game(ctx) == false)
		{
			continue;
		}

		// Skip data context
		if (ctx->user_name == nullptr)
		{
			continue;
		}

		// Skip if not on the same map
#if 0
		if( same_map_only )
		{
			if( target == nullptr )
			{
				continue;
			}
			if( strcmp(target->map,ctx->map) != 0 )
			{
				continue;
			}
		}
#endif
		network_send_text(ctx->id, text);
	} while ((ctx = ctx->next) != nullptr);

	context_unlock_list();
}

/*******************************************************************************
 Server send a user's character
 ******************************************************************************/
void network_send_user_character(context_t * p_pCtx,
		const char * p_pCharacterId, const char * p_pType, const char * p_pName)
{
	NetworkFrame l_Frame;
	l_Frame.push(p_pCharacterId);
	l_Frame.push(p_pType);
	l_Frame.push(p_pName);

	network_send_command(p_pCtx, CMD_SEND_USER_CHARACTER, l_Frame, false);
}

/*******************************************************************************
 Server send the full character's file to client
 ******************************************************************************/
void network_send_character_file(context_t * context)
{
	char * filename;

	filename = strconcat(CHARACTER_TABLE, "/", context->id, nullptr);
	network_send_file(context, filename);
	free(filename);
}

/*******************************************************************************
 Asks to update an int entry on all in_game players
 ******************************************************************************/
void network_broadcast_entry_int(const char * table, const char * file,
		const char * path, int value, bool same_map_only)
{
	context_t * ctx = nullptr;
	context_t * target = nullptr;

	target = context_find(file);

	context_lock_list();

	ctx = context_get_first();

	if (ctx == nullptr)
	{
		context_unlock_list();
		return;
	}

	do
	{
		if (context_is_npc(ctx) == true)
		{
			continue;
		}
		// Skip if not in game
		if (context_get_in_game(ctx) == false)
		{
			continue;
		}
		// Skip if not on the same map
		if (same_map_only == true)
		{
			if (target == nullptr)
			{
				continue;
			}
			if (target->map && ctx->map)
			{
				if (strcmp(target->map, ctx->map) != 0)
				{
					continue;
				}
			}
		}

		network_send_entry_int(ctx, table, file, path, value);

	} while ((ctx = ctx->next) != nullptr);

	context_unlock_list();
}

/*******************************************************************************
 ******************************************************************************/
static int new_connection(void * p_pData)
{
	context_t * l_pContext = nullptr;
	TCPsocket l_Socket = (TCPsocket) p_pData;

	l_pContext = context_new();
	if (l_pContext == nullptr)
	{
		werr(LOGUSER, "Failed to create l_pContext");
		return RET_NOK;
	}

	context_set_socket(l_pContext, l_Socket);

	context_new_VM(l_pContext);

	while (context_get_socket(l_pContext))
	{
		uint32_t l_FrameSize = 0U;

		if (network_read_bytes(l_Socket, (char *) &l_FrameSize,
				sizeof(uint32_t)) == RET_NOK)
		{
			break;
		}

		{
			l_FrameSize = ntohl(l_FrameSize);
			NetworkFrame l_Frame(l_FrameSize);

			if (network_read_bytes(l_Socket, (char *) l_Frame.getFrame(),
					l_FrameSize) == RET_NOK)
			{
				break;
			}
			if (parse_incoming_data(l_pContext, l_Frame) == RET_NOK)
			{
				break;
			}
		}
	}

	wlog(LOGUSER, "Client disconnected");
	context_spread(l_pContext);
	context_write_to_file(l_pContext);
	context_free(l_pContext);

	return RET_OK;
}

/*******************************************************************************
 ******************************************************************************/
void network_init(void)
{
	IPaddress IP;
	IPaddress * remote_IP;
	TCPsocket socket;
	TCPsocket client_socket;
	SDLNet_SocketSet server_set;

	if (SDLNet_Init() < 0)
	{
		werr(LOGUSER, "Can't init SDL: %s\n", SDLNet_GetError());
		return;
	}

	// Resolving the host using nullptr make network interface to listen
	if (SDLNet_ResolveHost(&IP, nullptr, PORT) < 0)
	{
		werr(LOGUSER, "Cannot listen on port %d: %s\n", PORT,
				SDLNet_GetError());
		return;
	}

	// Open a connection with the IP provided (listen on the host's port)
	if (!(socket = SDLNet_TCP_Open(&IP)))
	{
		werr(LOGUSER, "Cannot open port %d: %s\n", PORT, SDLNet_GetError());
		return;
	}

	server_set = SDLNet_AllocSocketSet(1);
	SDLNet_TCP_AddSocket(server_set, socket);

	// Wait for a connection
	while (true)
	{
		SDLNet_CheckSockets(server_set, -1);
		/* check for pending connection.
		 * If there is one, accept that, and open a new socket for communicating */
		if ((client_socket = SDLNet_TCP_Accept(socket)))
		{
			// Get the remote address
			if (!(remote_IP = SDLNet_TCP_GetPeerAddress(client_socket)))
			{
				werr(LOGUSER, "Can't get peer address: %s", SDLNet_GetError());
			}

			//wlog(LOGUSER,"Host connected: %s %d\n", SDLNet_Read32(&remote_IP->host), SDLNet_Read16(&remote_IP->port));
			wlog(LOGUSER, "Host connected: %x %d\n",
					SDLNet_Read32(&remote_IP->host),
					SDLNet_Read16(&remote_IP->port));

			SDL_CreateThread(new_connection, "new_connection",
					(void*) client_socket);
		}
	}

	SDLNet_TCP_Close(socket);
	SDLNet_Quit();

	return;
}

/*******************************************************************************
 Sends popup screen data to context
 dialog is a nullptr terminated array of string:
 "action" <action name> <param>  // if action_name is popup_end, this action close the pop-up
 "image" <image name>
 "text"  <text>
 "eol" end of line
 "eop" end of paragraph
 ******************************************************************************/
void network_send_popup(const std::string & p_rCtxId,
		const std::vector<std::string> & p_rPopupData)
{
	NetworkFrame l_Frame;
	l_Frame.push(p_rPopupData);

	wlog(LOGDEVELOPER, "Send CMD_SEND_POPUP : send pop-up to %s",
			p_rCtxId.c_str());

	context_t * l_pTarget = nullptr;
	l_pTarget = context_find(p_rCtxId.c_str());
	network_send_command(l_pTarget, CMD_SEND_POPUP, l_Frame, false);
}

/*******************************************************************************
 Broadcast effect
 p_Type is the effect's target (either a context or a map)
 p_TargetId is the name of the target (either a context ID or map ID)
 p_Parameters is an array of parameter string
 ******************************************************************************/
void network_broadcast_effect(EffectType p_Type, const std::string & p_TargetId,
		const std::vector<std::string> & p_Param)
{
	context_t * ctx = nullptr;

	context_lock_list();

	ctx = context_get_first();

	if (ctx == nullptr)
	{
		context_unlock_list();
		return;
	}

	std::string l_TargetMap = "";
	switch (p_Type)
	{
	case EffectType::CONTEXT:
		l_TargetMap = ctx->map;
		break;
	case EffectType::MAP:
		l_TargetMap = p_TargetId;
		break;
	default:
		werr(LOGDESIGNER, "network_broadcast_effect: Unknown EffectType");
		return;
		break;
	}

	NetworkFrame l_Frame;
	l_Frame.push(p_Param);

	do
	{
		if (ctx->map == nullptr)
		{
			continue;
		}

		if (context_is_npc(ctx) == true)
		{
			continue;
		}

		// Skip if not in game
		if (context_get_in_game(ctx) == false)
		{
			continue;
		}

		std::string l_CurrentMap = ctx->map;
		// Skip if not on the same map
		if (l_TargetMap != l_CurrentMap)
		{
			continue;
		}

		wlog(LOGDEVELOPER, "Send CMD_SEND_EFFECT to %s", ctx->id);
		network_send_command(ctx, CMD_SEND_EFFECT, l_Frame, false);
	} while ((ctx = ctx->next) != nullptr);

	context_unlock_list();
}

/*********************************************************************
 **********************************************************************/
void network_send_login_ok(context_t * context)
{
	pb::ServerMessage message;
	message.mutable_login_ok()->Clear();
	std::string serialized_data = message.SerializeAsString();

	NetworkFrame frame;
	frame.push(serialized_data);

	wlog(LOGDEVELOPER, "[network] Send LOGIN OK");
	network_send_command(context, CMD_PB, frame, false);
}

/*********************************************************************
 **********************************************************************/
void network_send_login_nok(context_t * context)
{
	pb::ServerMessage message;
	message.mutable_login_nok()->Clear();
	std::string serialized_data = message.SerializeAsString();

	NetworkFrame frame;
	frame.push(serialized_data);

	wlog(LOGDEVELOPER, "[network] Send LOGIN NOK");
	network_send_command(context, CMD_PB, frame, false);
}

/*********************************************************************
 **********************************************************************/
void network_send_playable_character(context_t * context,
		const std::vector<std::string> & id_list)
{
	pb::ServerMessage message;

	for (auto id : id_list)
	{
		message.mutable_playable_character()->add_id(id);
	}

	std::string serialized_data = message.SerializeAsString();

	NetworkFrame frame;
	frame.push(serialized_data);

	wlog(LOGDEVELOPER, "[network] Send LOGIN NOK");
	network_send_command(context, CMD_PB, frame, false);
}
