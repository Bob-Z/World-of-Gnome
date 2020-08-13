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

#ifndef COMMON_NETWORK_CONNECTION_H_
#define COMMON_NETWORK_CONNECTION_H_

#include "Lock.h"
#include <SDL2/SDL_net.h>
#include <string>

class Connection
{
public:
	Connection();
	virtual ~Connection();

	void send(const std::string m_serializedData, const bool m_isData);
	void disconnect();

	const std::string& getHostName() const;
	void setHostName(const std::string& hostName);

	TCPsocket getSocket() const;
	void setSocket(TCPsocket socket);

	TCPsocket getSocketData() const;
	void setSocketData(TCPsocket socketData);

	bool isConnected() const;
	void setConnected(bool connected);

	const std::string& getUserName() const;
	void setUserName(const std::string& userName);

	const std::string& getContextId() const;
	void setContextId(const std::string& contextId);

private:
	TCPsocket m_socket;
	Lock m_lockSend;
	TCPsocket m_socketData;
	Lock m_lockSendData;

	std::string m_hostName;
	std::string m_userName;

	bool m_connected; // User logged with the correct password, or NPC activated

	std::string m_contextId;
};

#endif /* COMMON_NETWORK_CONNECTION_H_ */
