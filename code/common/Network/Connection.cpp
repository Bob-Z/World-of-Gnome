/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2019 carabobz@gmail.com

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

#include "Connection.h"

#include "log.h"
#include "SdlLocking.h"
#include <arpa/inet.h>

/*****************************************************************************/
Connection::Connection() :
		m_socket(0), m_mutexSend(nullptr), m_socketData(0), m_mutexSendData(nullptr), m_hostName(), m_userName(), m_connected(false)
{
	m_mutexSend = SDL_CreateMutex();
	m_mutexSendData = SDL_CreateMutex();
}

/*****************************************************************************/
Connection::~Connection()
{
	disconnect();

	SDL_DestroyMutex(m_mutexSend);
	SDL_DestroyMutex(m_mutexSendData);
}

/*****************************************************************************/
const std::string& Connection::getHostName() const
{
	return m_hostName;
}

/*****************************************************************************/
void Connection::setHostName(const std::string& hostName)
{
	m_hostName = hostName;
}

/*****************************************************************************/
TCPsocket Connection::getSocket() const
{
	return m_socket;
}

/*****************************************************************************/
void Connection::setSocket(TCPsocket socket)
{
	m_socket = socket;
}

/*****************************************************************************/
TCPsocket Connection::getSocketData() const
{
	return m_socketData;
}

/*****************************************************************************/
void Connection::setSocketData(TCPsocket socketData)
{
	m_socketData = socketData;
}

/*****************************************************************************/
bool Connection::isConnected() const
{
	return m_connected;
}

/*****************************************************************************/
void Connection::setConnected(bool connected)
{
	m_connected = connected;
}

/*****************************************************************************/
const std::string& Connection::getUserName() const
{
	return m_userName;
}

/*****************************************************************************/
void Connection::setUserName(const std::string& userName)
{
	m_userName = userName;
}

/*****************************************************************************/
void Connection::send(const std::string m_serializedData, const bool m_isData)
{
	if (isConnected() == false)
	{
		return;
	}

	TCPsocket socket = 0;
	SDL_mutex * mutex = nullptr;

	if (m_isData == true)
	{
		//socket = getSocketData();  FIXME
		//mutex = m_mutexSendData;

		socket = getSocket();
		mutex = m_mutexSend;
	}
	else
	{
		socket = getSocket();
		mutex = m_mutexSend;
	}

	if (socket == 0)
	{
		wlog(LOGDEVELOPER, "socket %d is disconnected", socket);
		return;
	}

	SdlLocking lock(mutex);

	//send frame size
	uint32_t length = htonl(static_cast<uint32_t>(m_serializedData.size()));

	int l_BytesWritten = SDLNet_TCP_Send(socket, &length, sizeof(length));
	if (l_BytesWritten != sizeof(length))
	{
		werr(LOGUSER, "Could not send command to %s", getUserName().c_str());
		setConnected(false);
	}
	else
	{
		//wlog(LOGDEVELOPER, "sent %u bytes on socket %d", l_BytesWritten, l_Socket);

		//send frame
		l_BytesWritten = SDLNet_TCP_Send(socket, m_serializedData.c_str(), m_serializedData.size());
		if (l_BytesWritten != static_cast<int>(m_serializedData.size()))
		{
			werr(LOGUSER, "Could not send command to %s", getUserName().c_str());
			setConnected(false);
		}
	}

	//wlog(LOGDEVELOPER, "sent %u bytes on socket %d", l_BytesWritten, l_Socket);
}

/*****************************************************************************/
void Connection::disconnect()
{
	wlog(LOGDEVELOPER, "Disconnecting user %s", getUserName().c_str());

	setConnected(false);
	SDLNet_TCP_Close(m_socket);
	SDLNet_TCP_Close(m_socketData);
	setSocket(0);
	setSocketData(0);
}

/*****************************************************************************/
const std::string& Connection::getContextId() const
{
	return m_contextId;
}

/*****************************************************************************/
void Connection::setContextId(const std::string& contextId)
{
	wlog(LOGDEVELOPER, "User %s select ID %s", getUserName().c_str(), contextId.c_str());
	m_contextId = contextId;
}
