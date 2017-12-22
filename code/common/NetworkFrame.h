/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2017 carabobz@gmail.com

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
#include <string>
#include <vector>

class NetworkFrame
{
public:
	NetworkFrame(const size_t p_Size = 0U);
	virtual ~NetworkFrame();

	const char * getFrame() const;
	const size_t getSize() const;
	void push(const uint_fast32_t p_IntData);
	void push(const int_fast32_t p_IntData);
	void push(const int p_IntData);
	void push(const std::string& p_rStringData);
	void push(const std::vector<std::string> & p_rStringVectorData);
	void push(const char* p_rAsciiData);
	void push(const void* p_pBinaryData, const uint_fast32_t p_Size);
	void push(const NetworkFrame & p_rFrame);

	void pop(uint_fast32_t &p_rData);
	void pop(int_fast32_t &p_rData);
	void pop(std::string &p_rData);
	void pop(void* & p_rBinaryData, int_fast32_t & p_rSize);

private:
	void prepareFrame(const size_t p_AddedSizeData);
	void addData(const void * p_pData, const size_t p_Size);
	size_t readSize();

	char * m_pFrame;
	size_t m_Size;
	uint_fast32_t m_Index;
};

#endif // COMMON_NETWORKFRAME_H_
