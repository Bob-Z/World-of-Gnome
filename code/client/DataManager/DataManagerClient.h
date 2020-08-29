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

#ifndef CLIENT_DATAMANAGERCLIENT_H_
#define CLIENT_DATAMANAGERCLIENT_H_

#include "DataManager.h"
#include "FileReceivedObserver.h"
#include "FileStatus.h"
#include "log.h"
#include <string>
#include <unordered_map>
#include <vector>

class DataManagerClient: DataManager, FileReceivedObserver
{
public:
	DataManagerClient();
	virtual ~DataManagerClient();

	/*************************************************************************/
	template<typename T>
	T get(const std::string & table, const std::string & file, const std::vector<std::string> & resource)
	{
		std::string filePath = getFilePath(table, file);

		try
		{
			auto & status = m_timeStampPool.at(filePath);
			(void) status;
		} catch (...)
		{
			LOG("First access to file " + filePath + ", asking newest version");

			FileStatus newFileStatus;
			m_timeStampPool.insert(std::pair<std::string, FileStatus>(filePath, newFileStatus));
			requestFileFromServer(filePath);
		}

		try
		{
			return DataManager::get<T>(table, file, resource);
		} catch (...)
		{
			requestFileFromServer(filePath);

			throw;
		}
	}

	/*************************************************************************/
	template<typename T>
	T getNoExcept(const std::string & table, const std::string & file, const std::vector<std::string> & resource, const T & defaultValue)
	{
		try
		{
			return DataManagerClient::get<T>(table, file, resource);
		} catch (...)
		{
			return defaultValue;
		}
	}

	void fileReceived(const std::string & filePath) override;

private:
	void requestFileFromServer(const std::string & filePath);

	std::unordered_map<std::string, FileStatus> m_timeStampPool;
};

#endif /* CLIENT_DATAMANAGERCLIENT_H_ */
