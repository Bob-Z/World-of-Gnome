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

#include <bits/stdint-uintn.h>
#include <common.h>
#include <const.h>
#include <context.h>
#include <log.h>
#include <netinet/in.h>
#include <network.h>
#include <protocol.h>
#include <stdlib.h>
#include <syntax.h>
#include <SDL_net.h>
#include <SDL_thread.h>
#include <util.h>
#include <wog.pb.h>
#include <cstring>
#include <string>
#include <vector>

#include "../client/EffectManager.h"
#include "../client/parser.h"

/*********************************************************************
 *********************************************************************/
void network_send_text(const char * id, const std::string & string)
{
	context_t * context = context_find(id);
	if (context == nullptr)
	{
		werr(LOGDESIGNER, "Could not find context %s", id);
		return;
	}

	pb::ServerMessage message;
	message.mutable_text()->set_text(string);
	std::string serialized_data = message.SerializeAsString();

	wlog(LOGDEVELOPER, "[network] Send text to %s", id);
	network_send_command(context, serialized_data, false);
}

/*******************************************************************************
 Broadcast text to all in game players
 ******************************************************************************/
void network_broadcast_text(context_t * context, const std::string & text)
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
void network_send_user_character(context_t * context, const char * id,
		const char * type, const char * name)
{
	pb::ServerMessage message;
	message.mutable_user_character()->set_id(id);
	message.mutable_user_character()->set_type(type);
	message.mutable_user_character()->set_name(name);
	std::string serialized_data = message.SerializeAsString();

	wlog(LOGDEVELOPER, "[network] Send user %s character %s",
			context->user_name, name);
	network_send_command(context, serialized_data, false);
}

/*******************************************************************************
 Server send the full character's file to client
 ******************************************************************************/
void network_send_character_file(context_t * context)
{
	const std::string file_name = std::string(CHARACTER_TABLE) + "/"
			+ std::string(context->id);

	network_send_file(context, file_name.c_str());
}

/*******************************************************************************
 Asks to update an int entry on  a context
 ******************************************************************************/
void network_send_entry_int(context_t * context, const char * table,
		const char * file, const char *path, int value)
{
	pb::ServerMessage message;
	message.mutable_entry()->set_type(ENTRY_TYPE_INT);
	message.mutable_entry()->set_table(table);
	message.mutable_entry()->set_file(file);
	message.mutable_entry()->set_path(path);
	message.mutable_entry()->set_value(std::to_string(value));

	std::string serialized_data = message.SerializeAsString();

	wlog(LOGDEVELOPER, "[network] Send entry %s/%s/%s = %d", table, file, path,
			value);
	network_send_command(context, serialized_data, false);
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
	context_t * context = nullptr;
	TCPsocket socket = (TCPsocket) p_pData;

	context = context_new();
	if (context == nullptr)
	{
		werr(LOGUSER, "Failed to create context");
		return RET_NOK;
	}

	context_set_socket(context, socket);

	context_new_VM(context);

	while (context_get_socket(context))
	{
		uint32_t frame_size = 0U;

		if (network_read_bytes(socket, (char *) &frame_size,
				sizeof(uint32_t)) == RET_NOK)
		{
			break;
		}

		{
			frame_size = ntohl(frame_size);
			char frame[frame_size];

			if (network_read_bytes(socket, (char *) frame,
					frame_size) == RET_NOK)
			{
				break;
			}

			std::string serialized_data(frame, frame_size);
			if (parse_incoming_data(context, serialized_data) == RET_NOK)
			{
				break;
			}
		}
	}

	wlog(LOGUSER, "Client disconnected");
	context_spread(context);
	context_write_to_file(context);
	context_free(context);

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
void network_send_popup(const std::string & context_id,
		const std::vector<std::string> & popup_data)
{
	context_t * context = context_find(context_id.c_str());
	if (context == nullptr)
	{
		werr(LOGDEVELOPER, "[network] No context with ID %s",
				context_id.c_str());
		return;
	}

	pb::ServerMessage message;

	for (auto data : popup_data)
	{
		message.mutable_popup()->add_data(data);
	}

	std::string serialized_data = message.SerializeAsString();

	wlog(LOGDEVELOPER, "[network] Send pop-up");
	network_send_command(context, serialized_data, false);
}

/*******************************************************************************
 Broadcast effect
 p_Type is the effect's target (either a context or a map)
 p_TargetId is the name of the target (either a context ID or map ID)
 p_Parameters is an array of parameter string
 ******************************************************************************/
void network_broadcast_effect(EffectManager::EffectType p_Type,
		const std::string & p_TargetId, const std::vector<std::string> & params)
{
	context_t * ctx = nullptr;

	context_lock_list();

	ctx = context_get_first();

	if (ctx == nullptr)
	{
		context_unlock_list();
		return;
	}

	std::string target_map;
	switch (p_Type)
	{
	case EffectManager::EffectType::CONTEXT:
		target_map = ctx->map;
		break;
	case EffectManager::EffectType::MAP:
		target_map = p_TargetId;
		break;
	default:
		werr(LOGDESIGNER, "network_broadcast_effect: Unknown EffectType");
		return;
		break;
	}

	pb::ServerMessage message;

	for (auto param : params)
	{
		message.mutable_effect()->add_param(param);
	}

	std::string serialized_data = message.SerializeAsString();

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

		if (context_get_in_game(ctx) == false)
		{
			continue;
		}

		std::string l_CurrentMap = ctx->map;
		if (target_map != l_CurrentMap)
		{
			continue;
		}

		wlog(LOGDEVELOPER, "[network] Send effect to %s", ctx->id);
		network_send_command(ctx, serialized_data, false);
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

	wlog(LOGDEVELOPER, "[network] Send LOGIN OK");
	network_send_command(context, serialized_data, false);
}

/*********************************************************************
 **********************************************************************/
void network_send_login_nok(context_t * context)
{
	pb::ServerMessage message;
	message.mutable_login_nok()->Clear();
	std::string serialized_data = message.SerializeAsString();

	wlog(LOGDEVELOPER, "[network] Send LOGIN NOK");
	network_send_command(context, serialized_data, false);
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

	wlog(LOGDEVELOPER, "[network] Send playable character list");
	network_send_command(context, serialized_data, false);
}

/*********************************************************************
 send a source context's data to a destination context
 *********************************************************************/
void network_send_context_to_context(context_t * dest_ctx, context_t * src_ctx)
{
	// Skip if destination context is an NPC
	if (context_is_npc(dest_ctx))
	{
		return;
	}
	// Source context is not ready yet
	if (src_ctx->in_game == 0)
	{
		return;
	}
	if (src_ctx->user_name == nullptr)
	{
		return;
	}

	pb::ServerMessage message;
	message.mutable_context()->set_user_name(src_ctx->user_name);
	message.mutable_context()->set_character_name(src_ctx->character_name);
	message.mutable_context()->set_npc(src_ctx->npc);
	message.mutable_context()->set_map(src_ctx->map);
	message.mutable_context()->set_in_game(src_ctx->in_game);
	message.mutable_context()->set_connected(src_ctx->connected);
	message.mutable_context()->set_tile_x(src_ctx->tile_x);
	message.mutable_context()->set_tile_y(src_ctx->tile_y);
	message.mutable_context()->set_type(src_ctx->type);
	message.mutable_context()->set_id(src_ctx->id);
	message.mutable_context()->mutable_selection()->set_id(
			src_ctx->selection.id);
	message.mutable_context()->mutable_selection()->set_map(
			src_ctx->selection.map);
	message.mutable_context()->mutable_selection()->set_map_coord_tx(
			src_ctx->selection.map_coord[0]);
	message.mutable_context()->mutable_selection()->set_map_coord_ty(
			src_ctx->selection.map_coord[1]);
	message.mutable_context()->mutable_selection()->set_equipment(
			src_ctx->selection.equipment);
	message.mutable_context()->mutable_selection()->set_inventory(
			src_ctx->selection.inventory);

	std::string serialized_data = message.SerializeAsString();

	wlog(LOGDEVELOPER, "[network] Send context of %s to %s", src_ctx->id,
			dest_ctx->id);
	network_send_command(dest_ctx, serialized_data, false);
}
