/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2016 carabobz@gmail.com

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

#ifndef COMMON_NETWORKFRAME_H_
#define COMMON_NETWORKFRAME_H_

#include <cstdint>

class NetworkFrame
{
public:
	NetworkFrame();
	virtual ~NetworkFrame();

	const uint8_t * getFrame() const;
	const size_t getSize() const;
	void push(const uint_fast32_t p_IntData);
	void push(const std::string& p_rStringData);
	void push(const std::vector<std::string> & p_rStringVectorData);
	void push(const char* p_rAsciiData);
	void push(const NetworkFrame & p_rFrame);

private:
	void prepareFrame(const size_t p_AddedSizeData);
	void addData(const void * p_pData, const size_t p_Size);

	uint8_t * m_pFrame;
	size_t m_Size;
};

#endif /* COMMON_NETWORKFRAME_H_ */
