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

#include "DataSent.h"

#include "Connection.h"
#include "log.h"

/*****************************************************************************/
DataSent::DataSent() :
		m_connection(nullptr), m_serializedData(), m_isData(false)
{
}

/*****************************************************************************/
void DataSent::send()
{
	if (m_connection == nullptr)
	{
		werr(LOGDEVELOPER, "null connection");
	}

	m_connection->send(m_serializedData, m_isData);
}

/*****************************************************************************/
void DataSent::setConnection(Connection* connection)
{
	m_connection = connection;
}

/*****************************************************************************/
bool DataSent::isIsData() const
{
	return m_isData;
}

/*****************************************************************************/
void DataSent::setIsData(bool isData)
{
	m_isData = isData;
}

/*****************************************************************************/
const std::string& DataSent::getSerializedData() const
{
	return m_serializedData;
}

/*****************************************************************************/
void DataSent::setSerializedData(const std::string& serializedData)
{
	m_serializedData = serializedData;
}
