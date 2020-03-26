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

#include "const.h"
#include "log.h"
#include "network.h"
#include "screen.h"
#include "wog.pb.h"
#include <arpa/inet.h>

/*********************************************************************
 sends a login request, the answer is asynchronously read by async_recv
 **********************************************************************/
void network_login(Connection & connection, const std::string & user_name, const std::string & password)
{
	pb::ClientMessage message;
	message.mutable_login()->set_user(user_name);
	message.mutable_login()->set_password(password);
	const std::string serialized_data = message.SerializeAsString();

	LOG("[network] Send LOGIN");
	network_send_command(connection, serialized_data, false);
}

/*********************************************************************
 **********************************************************************/
void network_request_start(Connection & connection, const std::string & id)
{
	pb::ClientMessage message;
	message.mutable_start()->set_id(id);
	std::string serialized_data = message.SerializeAsString();

	LOG("[network] Send START");
	network_send_command(connection, serialized_data, false);
}

/*********************************************************************
 **********************************************************************/
void network_request_stop(Connection & connection)
{
	pb::ClientMessage message;
	message.mutable_stop()->Clear();
	std::string serialized_data = message.SerializeAsString();

	LOG("[network] Send STOP");
	network_send_command(connection, serialized_data, false);
}

/*********************************************************************
 request all playable characters list
 *********************************************************************/
void network_request_playable_character_list(Connection & connection)
{
	pb::ClientMessage message;
	message.mutable_playable_character_list()->Clear();
	std::string serialized_data = message.SerializeAsString();

	LOG("[network] Send PLAYABLE_CHARACTER_LIST");
	network_send_command(connection, serialized_data, false);
}

/*********************************************************************
 request a specific user's characters list
 *********************************************************************/
void network_request_user_character_list(Connection & connection)
{
	pb::ClientMessage message;
	message.mutable_user_character_list()->set_user(connection.getUserName());
	std::string serialized_data = message.SerializeAsString();

	LOG("[network] Send USER_CHARACTER_LIST");
	network_send_command(connection, serialized_data, false);
}

/*********************************************************************
 request a character's creation
 *********************************************************************/
void network_request_character_creation(Connection & connection, const char * id, const char * name)
{
	pb::ClientMessage message;
	message.mutable_create()->set_id(id);
	message.mutable_create()->set_name(name);
	std::string serialized_data = message.SerializeAsString();

	wlog(LOGDEVELOPER, "[network] Send CREATE ID = %s, NAME = %s", id, name);
	network_send_command(connection, serialized_data, false);
}

/*********************************************************************
 Player sends an action to server
 *********************************************************************/
void network_send_action(Connection & connection, const char * actionFile, ...)
{
	if (actionFile == nullptr)
	{
		werr(LOGDESIGNER, "Cannot ask for null action");
		return;
	}

	pb::ClientMessage message;
	message.mutable_action()->set_action(actionFile);

	va_list ap;
	char * parameter = nullptr;

	va_start(ap, actionFile);

	while ((parameter = va_arg(ap, char*)) != nullptr)
	{
		message.mutable_action()->add_params(parameter);
	}

	va_end(ap);

	std::string serialized_data = message.SerializeAsString();

	wlog(LOGDEVELOPER, "[network] Send action %s", actionFile);
	network_send_command(connection, serialized_data, false);
}

/*********************************************************************
 Callback from client listening to server in its own thread
 only used for game information transfer
 *********************************************************************/
static int async_recv(void * data)
{
	Connection * connection = (Connection *) data;

	while (true)
	{
		uint32_t frame_size = 0U;

		if (network_read_bytes(connection->getSocket(), (char *) &frame_size, sizeof(uint32_t)) == false)
		{
			break;
		}

		{
			frame_size = ntohl(frame_size);
			char frame[frame_size];

			if (network_read_bytes(connection->getSocket(), (char *) frame, frame_size) == false)
			{
				break;
			}

			std::string serialized_data(frame, frame_size);
			if (parse_incoming_data(*connection, serialized_data) == false)
			{
				break;
			}
		}
	}

	werr(LOGUSER, "Socket closed on server side.");

	connection->disconnect();

	screen_quit();

	return 0;
}

/*********************************************************************
 Callback from client listening to server in its own thread
 Only used for data transfers
 *********************************************************************/
static int async_data_recv(void * data)
{
	Connection * connection = (Connection *) data;

	while (true)
	{
		uint32_t frame_size = 0U;

		if (network_read_bytes(connection->getSocketData(), (char *) &frame_size, sizeof(uint32_t)) == false)
		{
			break;
		}

		{
			frame_size = ntohl(frame_size);
			char frame[frame_size];

			if (network_read_bytes(connection->getSocketData(), (char *) frame, frame_size) == false)
			{
				break;
			}

			std::string serialized_data(frame, frame_size);
			if (parse_incoming_data(*connection, serialized_data) == false)
			{
				break;
			}
		}
	}

	werr(LOGUSER, "Socket closed on server side.");

	connection->disconnect();

	screen_quit();

	return 0;
}

/*********************************************************************
 return false on error
 *********************************************************************/
int network_connect(Connection & connection, const std::string & host_name)
{
	IPaddress ip;
	TCPsocket socket;

	LOG_USER("Trying to connect to " + host_name + ":" + std::to_string(PORT));

	if (SDLNet_Init() < 0)
	{
		werr(LOGUSER, "Can't init SDLNet: %s\n", SDLNet_GetError());
		return false;
	}

	if (SDLNet_ResolveHost(&ip, host_name.c_str(), PORT) < 0)
	{
		werr(LOGUSER, "Can't resolve %s:%d : %s\n", host_name.c_str(), PORT, SDLNet_GetError());
		return false;
	}

	if ((socket = SDLNet_TCP_Open(&ip)) == 0)
	{
		werr(LOGUSER, "Can't connect to %s:%d : %s\n", host_name.c_str(), PORT, SDLNet_GetError());
		return false;
	}

	LOG_USER("Connected to " + host_name + ":" + std::to_string(PORT));

	connection.setHostName(host_name);
	connection.setSocket(socket);
	connection.setConnected(true);

	SDL_CreateThread(async_recv, "async_recv", (void*) &connection);

	return true;
}

/*****************************************************************************/
int network_open_data_connection(Connection & connection)
{
	IPaddress ip;
	TCPsocket socket = 0;

	if (SDLNet_ResolveHost(&ip, connection.getHostName().c_str(), PORT) < 0)
	{
		ERR_USER("Can't resolve " + connection.getHostName() + ":" + std::to_string(PORT) + " : " + std::string(SDLNet_GetError()));
		return false;
	}

	if ((socket = SDLNet_TCP_Open(&ip)) == 0)
	{
		ERR_USER("Can't open data connection to " + connection.getHostName() + ":" + std::to_string(PORT) + " : " + std::string(SDLNet_GetError()));
		return false;
	}

	connection.setSocketData(socket);

	SDL_CreateThread(async_data_recv, "async_data_recv", (void*) &connection);

	return true;
}
