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
#include "screen.h"
#include "wog.pb.h"
#include "log.h"
#include "const.h"
#include "network.h"
#include <arpa/inet.h>

/*********************************************************************
 sends a login request, the answer is asynchronously read by async_recv
 **********************************************************************/
void network_login(Context * context, const char * user_name, const char * password)
{
	pb::ClientMessage message;
	message.mutable_login()->set_user(user_name);
	message.mutable_login()->set_password(password);
	std::string serialized_data = message.SerializeAsString();

	wlog(LOGDEVELOPER, "[network] Send LOGIN");
	network_send_command(context, serialized_data, false);
}

/*********************************************************************
 **********************************************************************/
void network_request_start(Context * context, const std::string & id)
{
	pb::ClientMessage message;
	message.mutable_start()->set_id(id);
	std::string serialized_data = message.SerializeAsString();

	wlog(LOGDEVELOPER, "[network] Send START");
	network_send_command(context, serialized_data, false);
}

/*********************************************************************
 **********************************************************************/
void network_request_stop(Context * context)
{
	pb::ClientMessage message;
	message.mutable_stop()->Clear();
	std::string serialized_data = message.SerializeAsString();

	wlog(LOGDEVELOPER, "[network] Send STOP");
	network_send_command(context, serialized_data, false);
}

/*********************************************************************
 request all playable characters list
 *********************************************************************/
void network_request_playable_character_list(Context * context)
{
	pb::ClientMessage message;
	message.mutable_playable_character_list()->Clear();
	std::string serialized_data = message.SerializeAsString();

	wlog(LOGDEVELOPER, "[network] Send PLAYABLE_CHARACTER_LIST");
	network_send_command(context, serialized_data, false);
}

/*********************************************************************
 request a specific user's characters list
 *********************************************************************/
void network_request_user_character_list(Context * context)
{
	pb::ClientMessage message;
	message.mutable_user_character_list()->set_user(context->getUserName());
	std::string serialized_data = message.SerializeAsString();

	wlog(LOGDEVELOPER, "[network] Send USER_CHARACTER_LIST");
	network_send_command(context, serialized_data, false);
}

/*********************************************************************
 request a character's creation
 *********************************************************************/
void network_request_character_creation(Context * context, const char * id, const char * name)
{
	pb::ClientMessage message;
	message.mutable_create()->set_id(id);
	message.mutable_create()->set_name(name);
	std::string serialized_data = message.SerializeAsString();

	wlog(LOGDEVELOPER, "[network] Send CREATE ID = %s, NAME = %s", id, name);
	network_send_command(context, serialized_data, false);
}

/*********************************************************************
 Player sends an action to server
 *********************************************************************/
void network_send_action(Context * context, const char * script, ...)
{
	if (script == nullptr)
	{
		werr(LOGDESIGNER, "Cannot ask for null script");
		return;
	}

	pb::ClientMessage message;
	message.mutable_action()->set_script(script);

	va_list ap;
	char * parameter = nullptr;

	va_start(ap, script);

	while ((parameter = va_arg(ap, char*)) != nullptr)
	{
		message.mutable_action()->add_params(parameter);
	}

	va_end(ap);

	std::string serialized_data = message.SerializeAsString();

	wlog(LOGDEVELOPER, "[network] Send action script %s", script);
	network_send_command(context, serialized_data, false);
}

/*********************************************************************
 Callback from client listening to server in its own thread
 only used for game information transfer
 *********************************************************************/
static int async_recv(void * data)
{
	Context * context = (Context *) data;

	while (true)
	{
		uint32_t frame_size = 0U;

		if (network_read_bytes(context->m_socket, (char *) &frame_size, sizeof(uint32_t)) == RET_NOK)
		{
			break;
		}

		{
			frame_size = ntohl(frame_size);
			char frame[frame_size];

			if (network_read_bytes(context->m_socket, (char *) frame, frame_size) == RET_NOK)
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

	werr(LOGUSER, "Socket closed on server side.");

	context->setConnected(false);

	SDLNet_TCP_Close(context->m_socket);
	SDLNet_TCP_Close(context->m_socket_data);
	context_set_socket(context, 0);
	context_set_socket_data(context, 0);

	screen_quit();

	return 0;
}

/*********************************************************************
 Callback from client listening to server in its own thread
 Only used for data transfers
 *********************************************************************/
static int async_data_recv(void * data)
{
	Context * context = (Context *) data;

	while (true)
	{
		uint32_t frame_size = 0U;

		if (network_read_bytes(context->m_socket_data, (char *) &frame_size, sizeof(uint32_t)) == RET_NOK)
		{
			break;
		}

		{
			frame_size = ntohl(frame_size);
			char frame[frame_size];

			if (network_read_bytes(context->m_socket_data, (char *) frame, frame_size) == RET_NOK)
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

	werr(LOGUSER, "Socket closed on server side.");

	context->setConnected(false);
	SDLNet_TCP_Close(context->m_socket);
	SDLNet_TCP_Close(context->m_socket_data);
	context_set_socket(context, 0);
	context_set_socket_data(context, 0);

	screen_quit();

	return 0;
}

/*********************************************************************
 return RET_NOK on error
 *********************************************************************/
int network_connect(Context * context, const char * hostname)
{
	IPaddress ip;
	TCPsocket socket;

	wlog(LOGUSER, "Trying to connect to %s:%d", hostname, PORT);

	if (SDLNet_Init() < 0)
	{
		werr(LOGUSER, "Can't init SDLNet: %s\n", SDLNet_GetError());
		return RET_NOK;
	}

	if (SDLNet_ResolveHost(&ip, hostname, PORT) < 0)
	{
		werr(LOGUSER, "Can't resolve %s:%d : %s\n", hostname, PORT, SDLNet_GetError());
		return RET_NOK;
	}

	if (!(socket = SDLNet_TCP_Open(&ip)))
	{
		werr(LOGUSER, "Can't connect to %s:%d : %s\n", hostname, PORT, SDLNet_GetError());
		return RET_NOK;
	}

	wlog(LOGUSER, "Connected to %s:%d", hostname, PORT);

	context_set_hostname(context, hostname);
	context_set_socket(context, socket);

	SDL_CreateThread(async_recv, "async_recv", (void*) context);

	return RET_OK;
}

/*********************************************************************
 *********************************************************************/
int network_open_data_connection(Context * context)
{
	IPaddress ip;
	TCPsocket socket;

	if (SDLNet_ResolveHost(&ip, context->m_hostname, PORT) < 0)
	{
		werr(LOGUSER, "Can't resolve %s:%d : %s\n", context->m_hostname, PORT, SDLNet_GetError());
		return RET_NOK;
	}

	if (!(socket = SDLNet_TCP_Open(&ip)))
	{
		werr(LOGUSER, "Can't open data connection to %s:%d : %s\n", context->m_hostname, PORT, SDLNet_GetError());
		return RET_NOK;
	}

	context_set_socket_data(context, socket);

	SDL_CreateThread(async_data_recv, "async_data_recv", (void*) context);

	return RET_OK;
}
