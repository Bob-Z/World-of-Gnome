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

#include "DataManagerClient.h"

#include "const.h"
#include "Context.h"
#include "log.h"
#include "network.h"

/*****************************************************************************/
DataManagerClient::DataManagerClient() :
		m_timeStampPool()
{
}

/*****************************************************************************/
DataManagerClient::~DataManagerClient()
{
}

/*****************************************************************************/
void DataManagerClient::requestFileFromServer(const std::string & filePath)
{
	auto & fileStatus = m_timeStampPool.at(filePath);

	// Avoid flooding the server
	Uint32 currentTime = SDL_GetTicks();
	if ((fileStatus.getTimeStamp() != 0) && (fileStatus.getTimeStamp() + FILE_REQUEST_TIMEOUT_MS > currentTime))
	{
		LOG("Previous request of file " + filePath + " has been " + std::to_string(currentTime - fileStatus.getTimeStamp()) + " ms ago");
		return;
	}

	Context * context = context_get_player();
	if (context == nullptr)
	{
		ERR("Cannot retrieve player context");
		return;
	}

	Connection * connection = context->getConnection();
	if (connection == nullptr)
	{
		ERR("Cannot retrieve player context connection");
		return;
	}

	network_send_req_file(*connection, filePath);

	fileStatus.setTimeStamp(currentTime);
}
