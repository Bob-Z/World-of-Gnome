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

#ifndef COMMON_NETWORK_DATASENT_H_
#define COMMON_NETWORK_DATASENT_H_

#include <string>

class Connection;

class DataSent
{
public:
	DataSent();
	virtual ~DataSent() = default;

	void send();

	void setConnection(Connection* connection);

	bool isIsData() const;
	void setIsData(bool isData);

	const std::string& getSerializedData() const;
	void setSerializedData(const std::string& serializedData);

private:
	Connection * m_connection;
	std::string m_serializedData;
	bool m_isData;
};

#endif /* COMMON_NETWORK_DATASENT_H_ */
