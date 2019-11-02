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
#include "client_server.h"
#include "Context.h"
#include "file.h"
#include "log.h"
#include <netinet/in.h>
#include <SDL_mutex.h>
#include <SDL_net.h>
#include <SDL_thread.h>
#include "syntax.h"
#include "util.h"
#include <wog.pb.h>
#include <cstdint>
#include <cstring>
#include <string>
#include <utility>

// FIXME create NetworkManager
class DataSent
{
public:
	Context * m_pContext;
	std::string m_serialized_data;
	bool m_IsData;
};

/*********************************************************************
 return -1 on error
 return 0 on success
 *********************************************************************/
static int async_frame_send(void * p_pUserData)
{
	DataSent * data = static_cast<DataSent*>(p_pUserData);

	Context * context = data->m_pContext;
	if (context == nullptr)
	{
		werr(LOGDEVELOPER, "null l_pContext");
		delete data;
		return -1;
	}

	TCPsocket l_Socket = 0;

	if (data->m_IsData)
	{
		l_Socket = context_get_socket_data(context);
	}
	else
	{
		l_Socket = context_get_socket(context);
	}

	if (l_Socket == 0)
	{
		wlog(LOGDEVELOPER, "socket %d is disconnected", l_Socket);
		delete data;
		return -1;
	}

	SDL_LockMutex(context->m_send_mutex);

	//send frame size
	uint32_t length = htonl(static_cast<uint32_t>(data->m_serialized_data.size()));

	int l_BytesWritten = SDLNet_TCP_Send(l_Socket, &length, sizeof(length));
	if (l_BytesWritten != sizeof(length))
	{
		werr(LOGUSER, "Could not send command to %s", context->getId().c_str());
		context->setConnected(false);
		goto async_frame_send_end;
	}

	//wlog(LOGDEVELOPER, "sent %u bytes on socket %d", l_BytesWritten, l_Socket);

	//send frame
	l_BytesWritten = SDLNet_TCP_Send(l_Socket, data->m_serialized_data.c_str(), data->m_serialized_data.size());
	if (l_BytesWritten != static_cast<int>(data->m_serialized_data.size()))
	{
		werr(LOGUSER, "Could not send command to %s", context->getId().c_str());
		context->setConnected(false);
	}

	//wlog(LOGDEVELOPER, "sent %u bytes on socket %d", l_BytesWritten, l_Socket);

	async_frame_send_end: SDL_UnlockMutex(context->m_send_mutex);
	delete data;

	return true;
}

/*******************************************************************************
 ******************************************************************************/
void network_send_command(Context * context, const std::string & serialized_data, const bool is_data)
{
	// FIXME create a NetworkManager
	DataSent *data = new (DataSent);

	data->m_pContext = context;
	data->m_serialized_data = serialized_data;
	data->m_IsData = is_data;

	SDL_CreateThread(async_frame_send, "async_frame_send", (void*) data);
}

/*********************************************************************
 Client request a file
 It adds the local file checksum so that the server only send the file if it is different
 It make sure there are a minimum time between to consecutive request on the same file
 *********************************************************************/
void network_send_req_file(Context * context, const std::string & file_name)
{
	wlog(LOGDEVELOPER, "[network] Send request for file : %s", file_name.c_str());

	// Compute checksum of local file
	const std::string file_path = std::string(base_directory) + "/" + file_name;

	std::pair<bool, std::string> crc = checksum_file(file_path);
	if (crc.first == false)
	{
		crc.second = "0";
	}

	pb::ClientMessage message;
	message.mutable_file()->set_name(file_name);
	message.mutable_file()->set_crc(crc.second);
	std::string serialized_data = message.SerializeAsString();

	wlog(LOGDEVELOPER, "[network] Send FILE request for file : %s", file_name.c_str());
	// FIXME should last arg should be true
	network_send_command(context, serialized_data, false);
}

/*********************************************************************
 Return false on error, true on OK
 *********************************************************************/
int network_read_bytes(TCPsocket socket, char * data, int size)
{
	int bytes_read = 0;
	int total_bytes = 0;

	if (socket == 0)
	{
		return false;
	}

	while (total_bytes != size && bytes_read != -1)
	{
		bytes_read = SDLNet_TCP_Recv(socket, data + total_bytes, 1);
		if (bytes_read < 1)
		{
			werr(LOGDEVELOPER, "Read error on socket %d", socket);
			return false;
		}
		total_bytes += bytes_read;
	}

	//wlog(LOGDEVELOPER, "read %u bytes on socket %d", total_bytes, socket);

	return true;
}

/*********************************************************************
 **********************************************************************/
void network_send_file_data(Context * context, const std::string & name, const std::string & data)
{
	pb::ServerMessage message;
	message.mutable_file()->set_name(name);
	message.mutable_file()->set_data(data);
	std::string serialized_data = message.SerializeAsString();

	wlog(LOGDEVELOPER, "[network] Send file data for %s", name.c_str());
	network_send_command(context, serialized_data, false);
}

/*********************************************************************
 filename is relative to the data dir

 send a file to a context
 return 0 on success
 *********************************************************************/
int network_send_file(Context * context, const char * file_name)
{
	// Check if NPC
	if (context_is_npc(context) == true)
	{
		return -1;
	}

	// Never send files with password
	if (strstr(file_name, PASSWD_TABLE) != nullptr)
	{
		werr(LOGUSER, "send_file : Do not serve personal file  \"%s\"", file_name);
		return -1;
	}

	// Read the file
	void * file_data = nullptr;
	int_fast32_t file_length = 0;
	int res = 0;

	res = file_get_contents(file_name, &file_data, &file_length);
	if (res == false)
	{
		werr(LOGUSER, "send_file : Error reading file \"%s\"", file_name);
		return -1;
	}

	const std::string data(static_cast<char*>(file_data), file_length);

	network_send_file_data(context, file_name, data);

	return 0;
}

/*********************************************************************
 send table/file to a context
 return FALSE on success
 *********************************************************************/
int network_send_table_file(Context * context, const char * table, const char * id)
{
	const std::string file_name = std::string(table) + "/" + std::string(id);

	return network_send_file(context, file_name.c_str());
}
