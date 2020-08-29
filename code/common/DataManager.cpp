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

#include "client_server.h"
#include "DataManager.h"
#include "log.h"
#include <fstream>

/*****************************************************************************/
DataManager::DataManager() :
		m_jsonPool(), m_poolLock()
{
}

/*****************************************************************************/
DataManager::~DataManager()
{
}

/*****************************************************************************/
void DataManager::reset(const std::string & filePath)
{
	LockGuard guard(m_poolLock);

	try
	{
		m_jsonPool.erase(filePath);
		LOG("Remove JSON file " + filePath);
	} catch (...)
	{
		// filePath is not in JSON pool
	}
}

/*****************************************************************************/
std::string DataManager::getFilePath(const std::string & table, const std::string & file)
{
	std::string filePath;

	if (table.empty() == true)
	{
		filePath = file;
	}
	else
	{
		filePath = table + "/" + file;
	}

	return filePath;
}

/*****************************************************************************/
const json & DataManager::getJson(const std::string & filePath)
{
	try
	{
		return m_jsonPool.at(filePath);
	} catch (...)
	{
	}

	return loadJsonFile(filePath);
}

/*****************************************************************************/
const json & DataManager::loadJsonFile(const std::string & filePath)
{
	json object;
	std::ifstream stream(base_directory + "/" + filePath);
	stream >> object;

	m_jsonPool.insert(std::pair<std::string, json>(filePath, object));

	LOG("[DataManager] File " + filePath + " loaded");

	return m_jsonPool.at(filePath);
}
