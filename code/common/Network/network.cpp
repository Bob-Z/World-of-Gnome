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

#include "client_server.h"
#include "Context.h"
#include "DataSent.h"
#include "file.h"
#include "log.h"
#include "syntax.h"
#include "util.h"
#include <bits/stdint-uintn.h>
#include <LockGuard.h>
#include <cstdint>
#include <cstring>
#include <netinet/in.h>
#include <SDL_net.h>
#include <SDL_thread.h>
#include <string>
#include <utility>
#include <wog.pb.h>

/*********************************************************************
 return -1 on error
 return 0 on success
 *********************************************************************/
static int async_frame_send(void *user_data)
{
	DataSent *data = static_cast<DataSent*>(user_data);

	data->send();

	delete data;

	return true;
}

/*******************************************************************************
 ******************************************************************************/
void network_send_command(Connection &connection,
		const std::string &serialized_data, const bool is_data)
{
	DataSent *data = new (DataSent);

	data->setConnection(&connection);
	data->setSerializedData(serialized_data);
	data->setIsData(is_data);

	SDL_CreateThread(async_frame_send, "async_frame_send", (void*) data);
}

/*********************************************************************
 Client request a file
 It adds the local file checksum so that the server only send the file if it is different
 It make sure there are a minimum time between to consecutive request on the same file
 *********************************************************************/
void network_send_req_file(Connection &connection, const std::string &file_name)
{
	wlog(LOGDEVELOPER, "[network] Send request for file : %s",
			file_name.c_str());

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

	wlog(LOGDEVELOPER, "[network] Send FILE request for file : %s",
			file_name.c_str());

	network_send_command(connection, serialized_data, true);
}

/*********************************************************************
 Return false on error, true on OK
 *********************************************************************/
bool network_read_bytes(TCPsocket socket, char *data, int size)
{
	if (socket == 0)
	{
		return false;
	}

	int read = 0;
	int total = 0;

	while (total != size && read != -1)
	{
		read = SDLNet_TCP_Recv(socket, data + total, 1);
		if (read < 1)
		{
			ERR(
					"Read error " + std::to_string(read) + " on socket "
							+ std::to_string(reinterpret_cast<intptr_t>(socket)));
			return false;
		}
		total += read;
	}

	return true;
}

/*****************************************************************************/
void network_send_file_data(Connection &connection, const std::string &name,
		const std::string &data)
{
	pb::ServerMessage message;
	message.mutable_file()->set_name(name);
	message.mutable_file()->set_data(data);
	std::string serialized_data = message.SerializeAsString();

	wlog(LOGDEVELOPER, "[network] Send file data for %s", name.c_str());
	network_send_command(connection, serialized_data, false);
}

/*********************************************************************
 filename is relative to the data dir

 send a file to a connection
 return 0 on success
 *********************************************************************/
int network_send_file(Connection &connection, const char *file_name)
{
	// Never send files with password
	if (strstr(file_name, PASSWD_TABLE.c_str()) != nullptr)
	{
		werr(LOGUSER, "send_file : Do not serve personal file  \"%s\"",
				file_name);
		return -1;
	}

	// Read the file
	void *file_data = nullptr;
	int_fast32_t file_length = 0;
	int res = 0;

	res = file_get_contents(file_name, &file_data, &file_length);
	if (res == false)
	{
		werr(LOGUSER, "send_file : Error reading file \"%s\"", file_name);
		return -1;
	}

	const std::string data(static_cast<char*>(file_data), file_length);

	network_send_file_data(connection, file_name, data);

	return 0;
}

/*********************************************************************
 send table/file to a context
 return FALSE on success
 *********************************************************************/
int network_send_table_file(Context *context, const char *table, const char *id)
{
	const std::string file_name = std::string(table) + "/" + std::string(id);

	return network_send_file(*(context->getConnection()), file_name.c_str());
}
