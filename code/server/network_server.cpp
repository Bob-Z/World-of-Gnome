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

#include "Context.h"
#include <bits/stdint-uintn.h>
#include <const.h>
#include <cstring>
#include <log.h>
#include <netinet/in.h>
#include <network.h>
#include <protocol.h>
#include <SDL_net.h>
#include <SDL_thread.h>
#include <stdlib.h>
#include <string>
#include <syntax.h>
#include <util.h>
#include <vector>
#include <wog.pb.h>

#include "../client/EffectManager.h"
#include "network.h"

/*********************************************************************
 *********************************************************************/
void network_send_text(const std::string & id, const std::string & text)
{
	Context * context = context_find(id);
	if (context == nullptr)
	{
		werr(LOGDESIGNER, "Could not find context %s", id);
		return;
	}

	pb::ServerMessage message;
	message.mutable_text()->set_text(text);
	std::string serialized_data = message.SerializeAsString();

	wlog(LOGDEVELOPER, "[network] Send text to %s", id);
	network_send_command(context, serialized_data, false);
}

/*******************************************************************************
 Broadcast text to all in game players
 ******************************************************************************/
void network_broadcast_text(Context * context, const std::string & text)
{
	Context * ctx = nullptr;

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
		if (ctx->isInGame() == false)
		{
			continue;
		}

		// Skip data context
		if (ctx->getUserName() == "")
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
			if( strcmp(target->m_map,ctx->m_map) != 0 )
			{
				continue;
			}
		}
#endif
		network_send_text(ctx->getId(), text);
	} while ((ctx = ctx->m_next) != nullptr);

	context_unlock_list();
}

/*******************************************************************************
 Server send a user's character
 ******************************************************************************/
void network_send_user_character(Context * context, const char * id, const char * type, const char * name)
{
	pb::ServerMessage message;
	message.mutable_user_character()->set_id(id);
	message.mutable_user_character()->set_type(type);
	message.mutable_user_character()->set_name(name);
	std::string serialized_data = message.SerializeAsString();

	wlog(LOGDEVELOPER, "[network] Send user %s character %s", context->getUserName().c_str(), name);
	network_send_command(context, serialized_data, false);
}

/*******************************************************************************
 Server send the full character's file to client
 ******************************************************************************/
void network_send_character_file(Context * context)
{
	const std::string file_name = std::string(CHARACTER_TABLE) + "/" + context->getId();

	network_send_file(context, file_name.c_str());
}

/*******************************************************************************
 Asks to update an int entry on  a context
 ******************************************************************************/
void network_send_entry_int(Context * context, const char * table, const char * file, const char *path, int value)
{
	pb::ServerMessage message;
	message.mutable_entry()->set_type(ENTRY_TYPE_INT);
	message.mutable_entry()->set_table(table);
	message.mutable_entry()->set_file(file);
	message.mutable_entry()->set_path(path);
	message.mutable_entry()->set_value(std::to_string(value));

	std::string serialized_data = message.SerializeAsString();

	wlog(LOGDEVELOPER, "[network] Send entry %s/%s/%s = %d", table, file, path, value);
	network_send_command(context, serialized_data, false);
}

/*******************************************************************************
 Asks to update an int entry on all in_game players
 ******************************************************************************/
void network_broadcast_entry_int(const char * table, const char * file, const char * path, int value, bool same_map_only)
{
	Context * ctx = nullptr;
	Context * target = nullptr;

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
		if (ctx->isInGame() == false)
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
			if (target->getMap() != ctx->getMap())
			{
				continue;
			}
		}

		network_send_entry_int(ctx, table, file, path, value);

	} while ((ctx = ctx->m_next) != nullptr);

	context_unlock_list();
}

/*******************************************************************************
 ******************************************************************************/
static int new_connection(void * data)
{
	Context * context = nullptr;
	TCPsocket socket = (TCPsocket) data;

	context = context_new();
	if (context == nullptr)
	{
		werr(LOGUSER, "Failed to create context");
		return false;
	}

	context_set_socket(context, socket);

	while (context_get_socket(context))
	{
		uint32_t frame_size = 0U;

		if (network_read_bytes(socket, (char *) &frame_size, sizeof(uint32_t)) == false)
		{
			break;
		}

		{
			frame_size = ntohl(frame_size);
			char frame[frame_size];

			if (network_read_bytes(socket, (char *) frame, frame_size) == false)
			{
				break;
			}

			std::string serialized_data(frame, frame_size);
			if (parse_incoming_data(context, serialized_data) == false)
			{
				break;
			}
		}
	}

	wlog(LOGUSER, "Client disconnected");
	context_spread(context);
	context_write_to_file(context);
	context_free(context);

	return true;
}

/*****************************************************************************/
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
		werr(LOGUSER, "Cannot listen on port %d: %s\n", PORT, SDLNet_GetError());
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
			wlog(LOGUSER, "Host connected: %x %d\n", SDLNet_Read32(&remote_IP->host), SDLNet_Read16(&remote_IP->port));

			SDL_CreateThread(new_connection, "new_connection", (void*) client_socket);
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
void network_send_popup(const std::string & contextId, const std::vector<std::string> & popupData)
{
	Context * context = context_find(contextId.c_str());
	if (context == nullptr)
	{
		werr(LOGDEVELOPER, "[network] No context with ID %s", contextId.c_str());
		return;
	}

	pb::ServerMessage message;

	for (auto data : popupData)
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
void network_broadcast_effect(EffectManager::EffectType type, const std::string & targetId, const std::vector<std::string> & params)
{
	Context * ctx = nullptr;

	context_lock_list();

	ctx = context_get_first();

	if (ctx == nullptr)
	{
		context_unlock_list();
		return;
	}

	std::string target_map;
	switch (type)
	{
	case EffectManager::EffectType::CONTEXT:
		target_map = ctx->getMap();
		break;
	case EffectManager::EffectType::MAP:
		target_map = targetId;
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
		if (ctx->getMap() == "")
		{
			continue;
		}

		if (context_is_npc(ctx) == true)
		{
			continue;
		}

		if (ctx->isInGame() == false)
		{
			continue;
		}

		if (target_map != ctx->getMap())
		{
			continue;
		}

		wlog(LOGDEVELOPER, "[network] Send effect to %s", ctx->getId().c_str());
		network_send_command(ctx, serialized_data, false);
	} while ((ctx = ctx->m_next) != nullptr);

	context_unlock_list();
}

/*********************************************************************
 **********************************************************************/
void network_send_login_ok(Context * context)
{
	pb::ServerMessage message;
	message.mutable_login_ok()->Clear();
	std::string serialized_data = message.SerializeAsString();

	wlog(LOGDEVELOPER, "[network] Send LOGIN OK");
	network_send_command(context, serialized_data, false);
}

/*********************************************************************
 **********************************************************************/
void network_send_login_nok(Context * context)
{
	pb::ServerMessage message;
	message.mutable_login_nok()->Clear();
	std::string serialized_data = message.SerializeAsString();

	wlog(LOGDEVELOPER, "[network] Send LOGIN NOK");
	network_send_command(context, serialized_data, false);
}

/*********************************************************************
 **********************************************************************/
void network_send_playable_character(Context * context, const std::vector<std::string> & id_list)
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
void network_send_context_to_context(Context * dest_ctx, Context * src_ctx)
{
	// Skip if destination context is an NPC
	if (context_is_npc(dest_ctx))
	{
		return;
	}
	// Source context is not ready yet
	if (src_ctx->isInGame() == 0)
	{
		return;
	}
//	if (src_ctx->m_userName == nullptr)
//	{
//		return;
//	}

	pb::ServerMessage message;
	message.mutable_context()->set_user_name(src_ctx->getUserName());
	message.mutable_context()->set_character_name(src_ctx->getCharacterName());
	message.mutable_context()->set_npc(src_ctx->isNpc());
	message.mutable_context()->set_map(src_ctx->getMap());
	message.mutable_context()->set_in_game(src_ctx->isInGame());
	message.mutable_context()->set_connected(src_ctx->isConnected());
	message.mutable_context()->set_tile_x(src_ctx->getTileX());
	message.mutable_context()->set_tile_y(src_ctx->getTileY());
	message.mutable_context()->set_type(src_ctx->getType());
	message.mutable_context()->set_id(src_ctx->getId());
	message.mutable_context()->mutable_selection()->set_id(src_ctx->getSelectionContextId());
	message.mutable_context()->mutable_selection()->set_map(src_ctx->getSelectionMap());
	message.mutable_context()->mutable_selection()->set_map_coord_tx(src_ctx->getSelectionMapTx());
	message.mutable_context()->mutable_selection()->set_map_coord_ty(src_ctx->getSelectionMapTy());
	message.mutable_context()->mutable_selection()->set_equipment(src_ctx->getSelectionEquipment());
	message.mutable_context()->mutable_selection()->set_inventory(src_ctx->getSelectionInventory());

	std::string serialized_data = message.SerializeAsString();

	wlog(LOGDEVELOPER, "[network] Send context of %s to %s", src_ctx->getId().c_str(), dest_ctx->getId().c_str());
	network_send_command(dest_ctx, serialized_data, false);
}
