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

#include "client_server.h"
#include "DataManager.h"
#include "file.h"
#include "log.h"
#include <fstream>
#include <string>

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
void DataManager::add(const std::string & table, const std::string & file, const std::vector<std::string> & resource, const std::string & toAdd)
{
	std::string filePath = getFilePath(table, file);

	LockGuard guard(m_poolLock);

	auto jsonFile = getJson(filePath);
	auto reference = jsonFile;

	for (auto & res : resource)
	{
		reference = reference.at(res);
	}

	reference.push_back(toAdd);

	file_write(filePath, jsonFile.dump(2));
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
json & DataManager::getJson(const std::string & filePath)
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
json & DataManager::loadJsonFile(const std::string & filePath)
{
	json object;
	std::ifstream stream(base_directory + "/" + filePath);
	stream >> object;

	m_jsonPool.insert(std::pair<std::string, json>(filePath, object));

	LOG("[DataManager] File " + filePath + " loaded");

	return m_jsonPool.at(filePath);
}
