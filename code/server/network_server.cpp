/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2020 carabobz@gmail.com

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
#include <const.h>
#include "Context.h"
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
		ERR_DESIGN("Could not find context " + id);
		return;
	}

	if (context->getConnection() == nullptr)
	{
		ERR("No connection for context " + id);
		return;
	}

	pb::ServerMessage message;
	message.mutable_text()->set_text(text);
	std::string serialized_data = message.SerializeAsString();

	LOG("[network] Send text to " + id);
	network_send_command(*(context->getConnection()), serialized_data, false);
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
		if (ctx->isNpc() == true)
		{
			continue;
		}

		// Skip if not in game
		if (ctx->isInGame() == false)
		{
			continue;
		}

		// Skip data context
		if (ctx->getConnection()->getUserName() == "")
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
void network_send_user_character(Connection & connection, const std::string & id, const std::string & type, const std::string & name)
{
	pb::ServerMessage message;
	message.mutable_user_character()->set_id(id);
	message.mutable_user_character()->set_type(type);
	message.mutable_user_character()->set_name(name);
	std::string serialized_data = message.SerializeAsString();

	LOG("[network] Send user " + connection.getUserName() + " character " + name);
	network_send_command(connection, serialized_data, false);
}

/*******************************************************************************
 Server send the full character's file to client
 ******************************************************************************/
void network_send_character_file(Context * context)
{
	const std::string file_name = CHARACTER_TABLE + "/" + context->getId();

	network_send_file(*(context->getConnection()), file_name);
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

	LOG("[network] Send entry " + std::string(table) + "/" + std::string(file) + "/" + std::string(path) + " = " + std::to_string(value));
	network_send_command(*(context->getConnection()), serialized_data, false);
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
		if (ctx->isNpc() == true)
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
	TCPsocket socket = (TCPsocket) data;

	Connection connection;

	connection.setSocket(socket);
	connection.setConnected(true);

	while (connection.isConnected() == true)
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
			if (parse_incoming_data(connection, serialized_data) == false)
			{
				break;
			}
		}
	}

	std::string contextId = connection.getContextId();

	if (contextId.empty() == false)
	{
		Context * context = context_find(contextId);
		if (context != nullptr)
		{
			context_spread(context);
			context_write_to_file(context);
			context_free(context);
		}
	}

	LOG_USER("Client disconnected");

	return 0;
}

/*****************************************************************************/
void network_init(void)
{
	IPaddress IP;
	IPaddress * remoteIp = nullptr;
	TCPsocket socket = 0;
	SDLNet_SocketSet server_set;

	if (SDLNet_Init() < 0)
	{
		ERR_USER("Can't init SDL: " + std::string(SDLNet_GetError()) + "\n");
		return;
	}

	// Resolving the host using nullptr make network interface to listen
	if (SDLNet_ResolveHost(&IP, nullptr, PORT) < 0)
	{
		ERR_USER("Cannot listen on port " + std::to_string(PORT) + ": " + std::string(SDLNet_GetError()) + "\n");
		return;
	}

	// Open a connection with the IP provided (listen on the host's port)
	if ((socket = SDLNet_TCP_Open(&IP)) == 0)
	{
		ERR_USER("Cannot open port " + std::to_string(PORT) + ": " + std::string(SDLNet_GetError()) + "\n");
		return;
	}

	server_set = SDLNet_AllocSocketSet(1);
	SDLNet_TCP_AddSocket(server_set, socket);

	LOG_USER("Waiting for connections on port: " + std::to_string(PORT));

	// Wait for a connection
	while (true)
	{
		SDLNet_CheckSockets(server_set, -1);
		/* check for pending connection.
		 * If there is one, accept that, and open a new socket for communicating */
		TCPsocket clientSocket = 0;
		if ((clientSocket = SDLNet_TCP_Accept(socket)))
		{
			// Get the remote address
			if ((remoteIp = SDLNet_TCP_GetPeerAddress(clientSocket)) == nullptr)
			{
				ERR_USER("Can't get peer address: " + std::string(SDLNet_GetError()));
			}

			//LOG_USER("Host connected: " + std::string(SDLNet_Read32(&remote_IP->host)) + " " + std::to_string(SDLNet_Read16(&remote_IP->port)) + "\n");
			LOG_USER("Host connected: " + std::string(SDLNet_ResolveIP(remoteIp)) + ":" + std::to_string(SDLNet_Read16(&remoteIp->port)) + " with socket " + std::to_string(reinterpret_cast<intptr_t>(clientSocket)) + "\n");

			SDL_CreateThread(new_connection, "new_connection", (void*) clientSocket);
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
	Context * context = context_find(contextId);
	if (context == nullptr)
	{
		ERR("[network] No context with ID " + contextId);
		return;
	}

	pb::ServerMessage message;

	for (auto && data : popupData)
	{
		message.mutable_popup()->add_data(data);
	}

	std::string serialized_data = message.SerializeAsString();

	LOG("[network] Send pop-up");
	network_send_command(*(context->getConnection()), serialized_data, false);
}

/*******************************************************************************
 Broadcast effect
 p_Type is the effect's target (either a context or a map)
 p_TargetId is the name of the target (either a context ID or map ID)
 p_Parameters is an array of parameter string
 ******************************************************************************/
void network_broadcast_effect(EffectManager::EffectType type, const std::string & targetId, const std::vector<std::string> & paramArray)
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
		ERR_DESIGN("network_broadcast_effect: Unknown EffectType");
		return;
		break;
	}

	pb::ServerMessage message;

	for (auto && param : paramArray)
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

		if (ctx->isNpc() == true)
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

		LOG("[network] Send effect to " + ctx->getId());
		network_send_command(*(ctx->getConnection()), serialized_data, false);
	} while ((ctx = ctx->m_next) != nullptr);

	context_unlock_list();
}

/*********************************************************************
 **********************************************************************/
void network_send_login_ok(Connection & connection)
{
	pb::ServerMessage message;
	message.mutable_login_ok()->Clear();
	std::string serialized_data = message.SerializeAsString();

	LOG("[network] Send LOGIN OK");
	network_send_command(connection, serialized_data, false);
}

/*****************************************************************************/
void network_send_login_nok(Connection & connection)
{
	pb::ServerMessage message;
	message.mutable_login_nok()->Clear();
	std::string serialized_data = message.SerializeAsString();

	LOG("[network] Send LOGIN NOK");
	network_send_command(connection, serialized_data, false);
}

/*****************************************************************************/
void network_send_playable_character(Connection & connection, const std::vector<std::string> & idArray)
{
	pb::ServerMessage message;

	auto *playable = message.mutable_playable_character();
	for (auto && id : idArray)
	{
		playable->add_id(id);
	}

	std::string serialized_data = message.SerializeAsString();

	LOG("[network] Send " + std::to_string(idArray.size()) + " playable characters list");
	network_send_command(connection, serialized_data, false);
}

/******************************************************************************
 send a source context's data to a destination context
 *****************************************************************************/
void network_send_context_to_context(Context * dest_ctx, Context * src_ctx)
{
	if (dest_ctx->isNpc() == true)
	{
		return;
	}
	// Source context is not ready yet
	if (src_ctx->isInGame() == false)
	{
		return;
	}

	pb::ServerMessage message;
	message.mutable_context()->set_character_name(src_ctx->getCharacterName());
	message.mutable_context()->set_npc(src_ctx->isNpc());
	message.mutable_context()->set_map(src_ctx->getMap());
	message.mutable_context()->set_in_game(src_ctx->isInGame());
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

	LOG("[network] Send context of " + src_ctx->getId() + " to " + dest_ctx->getId());
	network_send_command(*(dest_ctx->getConnection()), serialized_data, false);
}
