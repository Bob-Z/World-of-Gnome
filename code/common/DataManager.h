/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2019-2020 carabobz@gmail.com

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

#ifndef COMMON_DATAMANAGER_H_
#define COMMON_DATAMANAGER_H_

#include "Lock.h"
#include "LockGuard.h"
#include "log.h"
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

using json = nlohmann::json;

class DataManager
{
public:
	DataManager();
	virtual ~DataManager();

	void add(const std::string & table, const std::string & file, const std::vector<std::string> & resource, const std::string & toAdd);

	/*************************************************************************/
	template<typename T>
	T get(const std::string & table, const std::string & file, const std::vector<std::string> & resource)
	{
		try
		{
			std::string filePath = getFilePath(table, file);

			LockGuard guard(m_poolLock);

			auto json = getJson(filePath);

			for (auto & res : resource)
			{
				LOG("resource " + res);
				json = json.at(res);
			}

			return json.get<T>();
		} catch (json::exception& e)
		{
			ERR("JSON : " + std::string(e.what()));
			throw;
		} catch (...)
		{
			ERR("JSON error");
			throw;
		}
	}

	/*************************************************************************/
	template<typename T>
	T getNoExcept(const std::string & table, const std::string & file, const std::vector<std::string> & resource, const T & defaultValue)
	{
		try
		{
			return get<T>(table, file, resource);
		} catch (...)
		{
			return defaultValue;
		}
	}

	void reset(const std::string & filePath);

protected:
	std::string getFilePath(const std::string & table, const std::string & file);

private:
	json & getJson(const std::string & filePath);
	json & loadJsonFile(const std::string & filePath);

	std::unordered_map<std::string, json> m_jsonPool;

	Lock m_poolLock;
};

#endif /* COMMON_DATAMANAGER_H_ */
