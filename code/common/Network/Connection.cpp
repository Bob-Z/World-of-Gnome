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

#include "LockGuard.h"
#include "log.h"
#include <arpa/inet.h>

/*****************************************************************************/
Connection::Connection() :
		m_socket(0), m_lockSend(), m_socketData(0), m_lockSendData(), m_hostName(), m_userName(), m_connected(false), m_socketReader(0), m_socketDataReader(0)
{
}

/*****************************************************************************/
Connection::~Connection()
{
	disconnect();
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
	Lock * lock = nullptr;

	if (m_isData == true)
	{
		//socket = getSocketData();  FIXME
		//mutex = m_mutexSendData;

		socket = getSocket();
		lock = &m_lockSend;
	}
	else
	{
		socket = getSocket();
		lock = &m_lockSend;
	}

	if (socket == 0)
	{
		LOG("socket " + std::to_string(reinterpret_cast<intptr_t>(socket)) + " is disconnected");
		return;
	}

	LockGuard guard(*lock);

	//send frame size
	uint32_t length = htonl(static_cast<uint32_t>(m_serializedData.size()));

	int l_BytesWritten = SDLNet_TCP_Send(socket, &length, sizeof(length));
	if (l_BytesWritten != sizeof(length))
	{
		ERR_USER("Could not send command to " + getUserName());
		setConnected(false);
	}
	else
	{
		//LOG("sent " + std::to_string(l_BytesWritten) + " bytes on socket " + std::to_string(l_Socket));

		//send frame
		l_BytesWritten = SDLNet_TCP_Send(socket, m_serializedData.c_str(), m_serializedData.size());
		if (l_BytesWritten != static_cast<int>(m_serializedData.size()))
		{
			ERR_USER("Could not send command to " + getUserName());
			setConnected(false);
		}
	}

	//LOG("sent " + std::to_string(l_BytesWritten) + " bytes on socket " + std::to_string(l_Socket));
}

/*****************************************************************************/
void Connection::setSocketReader(bool isData, std::thread::id readerThread)
{
	if (isData)
	{
		m_socketDataReader = readerThread;
	}
	else
	{
		m_socketReader = readerThread;
	}
}

/*****************************************************************************/
void Connection::closeSocket(bool isData)
{
	LockGuard guard(isData ? m_lockSendData : m_lockSend);

	TCPsocket &socket = isData ? m_socketData : m_socket;
	std::thread::id reader = isData ? m_socketDataReader : m_socketReader;

	/* A reader thread is still inside SDLNet_TCP_Recv() on this socket; only
	 * that thread may close it, otherwise we free memory it is using. */
	if (socket == nullptr
			|| (reader != std::thread::id {}
					&& reader != std::this_thread::get_id()))
	{
		return;
	}

	LOG("Disconnecting user " + getUserName());

	setConnected(false);
	SDLNet_TCP_Close(socket);
	socket = nullptr;
}

/*****************************************************************************/
void Connection::disconnect()
{
	closeSocket(false);
	closeSocket(true);
}

/*****************************************************************************/
const std::string& Connection::getContextId() const
{
	return m_contextId;
}

/*****************************************************************************/
void Connection::setContextId(const std::string& contextId)
{
	LOG("User " + getUserName() + " select ID " + contextId);
	m_contextId = contextId;
}
