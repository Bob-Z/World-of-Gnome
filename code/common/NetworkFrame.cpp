/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2019 carabobz@gmail.com

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

#include "common.h"
#include "NetworkFrame.h"
#include <arpa/inet.h>
#include <stdio.h>

/******************************************************************************/
NetworkFrame::NetworkFrame(const size_t p_Size) :
		m_pFrame(nullptr), m_Size(p_Size), m_Index(0U)
{
	if (m_Size != 0U)
	{
		m_pFrame = static_cast<char*>(malloc(m_Size));
	}
}

/******************************************************************************/
NetworkFrame::~NetworkFrame()
{
	if (m_pFrame != nullptr)
	{
		free(m_pFrame);
	}
}

/******************************************************************************/
const char * NetworkFrame::getFrame() const
{
	return m_pFrame;
}

/******************************************************************************/
const size_t NetworkFrame::getSize() const
{
	return m_Size;
}

/******************************************************************************/
void NetworkFrame::push(const uint_fast32_t p_IntData)
{
	prepareFrame(sizeof(uint32_t));

	uint32_t l_Data = htonl(static_cast<uint32_t>(p_IntData));

	addData(&l_Data, sizeof(l_Data));
}

/******************************************************************************/
void NetworkFrame::push(const int_fast32_t p_IntData)
{
	prepareFrame(sizeof(int32_t));

	int32_t l_Data = htonl(static_cast<int32_t>(p_IntData));

	addData(&l_Data, sizeof(l_Data));
}

/******************************************************************************/
void NetworkFrame::push(const int p_IntData)
{
	push(static_cast<int_fast32_t>(p_IntData));
}

/******************************************************************************/
void NetworkFrame::push(const bool p_BoolData)
{
	push(static_cast<int_fast32_t>(p_BoolData));
}

/******************************************************************************/
void NetworkFrame::push(const std::string& p_rStringData)
{
	prepareFrame(p_rStringData.size());
	addData(p_rStringData.c_str(), p_rStringData.size());
}

/******************************************************************************/
void NetworkFrame::push(const std::vector<std::string> & p_rStringVectorData)
{
	push(static_cast<uint_fast32_t>(p_rStringVectorData.size()));

	for (auto l_It = p_rStringVectorData.begin();
			l_It != p_rStringVectorData.end(); ++l_It)
	{
		prepareFrame(l_It->size());
		addData(l_It->c_str(), l_It->size());
	}
}

/******************************************************************************/
void NetworkFrame::push(const char* AsciiData)
{
	const char* NoData = "";
	const char* DataToSend = AsciiData;

	if (AsciiData == nullptr)
	{
		DataToSend = NoData;
	}

	size_t Size = strlen(DataToSend) + 1; //terminal null
	prepareFrame(Size);
	addData(AsciiData, Size);
}

/******************************************************************************/
void NetworkFrame::push(const void* p_pBinaryData, const uint_fast32_t p_Size)
{
	prepareFrame(p_Size);
	addData(p_pBinaryData, p_Size);
}

/******************************************************************************/
void NetworkFrame::push(const NetworkFrame & p_rFrame)
{
	size_t l_Size = p_rFrame.getSize();

	m_pFrame = static_cast<char*>(realloc(static_cast<void*>(m_pFrame),
			m_Size + l_Size));

	memcpy(&m_pFrame[m_Size], p_rFrame.m_pFrame, l_Size);
	m_Size += l_Size;
}

/******************************************************************************/
void NetworkFrame::pop(uint_fast32_t & p_rData)
{
	size_t l_Size = readSize();
	if (l_Size != sizeof(uint32_t))
	{
		werr(LOGDESIGNER, "Element size %d is wrong, should be %d", l_Size,
				sizeof(uint32_t));
		return;
	}

	if (m_Index >= m_Size)
	{
		werr(LOGDESIGNER, "Frame is empty.");
		return;
	}

	uint32_t l_Data = 0U;
	memcpy(&l_Data, &m_pFrame[m_Index], l_Size);
	m_Index += l_Size;

	p_rData = ntohl(l_Data);
}

/******************************************************************************/
void NetworkFrame::pop(int_fast32_t & p_rData)
{
	size_t l_Size = readSize();
	if (l_Size != sizeof(int32_t))
	{
		werr(LOGDESIGNER, "Element size %d is wrong, should be %d", l_Size,
				sizeof(uint32_t));
		return;
	}

	if (m_Index >= m_Size)
	{
		werr(LOGDESIGNER, "Frame is empty.");
		return;
	}

	int32_t l_Data = 0U;
	memcpy(&l_Data, &m_pFrame[m_Index], l_Size);
	m_Index += l_Size;

	p_rData = ntohl(l_Data);
}

/******************************************************************************/
void NetworkFrame::pop(int & p_rData)
{
	int_fast32_t l_Data;
	pop(l_Data);
	p_rData = static_cast<int>(l_Data);
}

/******************************************************************************/
void NetworkFrame::pop(bool & p_rData)
{
	int_fast32_t l_Data;
	pop(l_Data);
	p_rData = static_cast<int>(l_Data);
}

/******************************************************************************/
void NetworkFrame::pop(std::string & p_rData)
{
	size_t l_Size = readSize();

	if (m_Index >= m_Size)
	{
		werr(LOGDESIGNER, "Frame is empty.");
		return;
	}

	std::string l_Data(&m_pFrame[m_Index], l_Size);
	m_Index += l_Size;

	p_rData = l_Data;
}

/******************************************************************************/
void NetworkFrame::pop(std::vector<std::string> & p_rStringVectorData)
{
	uint_fast32_t l_VectorSize = 0U;

	pop(l_VectorSize);

	std::string l_ReadString;

	for (uint_fast32_t l_Index = 0U; l_Index < l_VectorSize; l_Index++)
	{
		pop(l_ReadString);
		p_rStringVectorData.push_back(l_ReadString);
	}
}

/******************************************************************************/
void NetworkFrame::pop(char* & p_rAsciiData)
{
	size_t l_Size;
	l_Size = readSize();

	p_rAsciiData = static_cast<char*>(malloc(l_Size));
	memcpy(static_cast<void*>(p_rAsciiData), &m_pFrame[m_Index], l_Size);
	m_Index += l_Size;
}

/******************************************************************************/
void NetworkFrame::pop(void* & p_rBinaryData, int_fast32_t & p_rSize)
{
	p_rSize = readSize();

	p_rBinaryData = malloc(p_rSize);
	memcpy(p_rBinaryData, &m_pFrame[m_Index], p_rSize);
	m_Index += p_rSize;
}

/******************************************************************************/
void NetworkFrame::prepareFrame(const size_t p_AddedSizeData)
{
	uint_fast32_t l_FrameSize = sizeof(uint32_t) + p_AddedSizeData;

	m_pFrame = static_cast<char*>(realloc(static_cast<void*>(m_pFrame),
			m_Size + l_FrameSize));

	uint32_t l_DataSize = htonl(static_cast<uint32_t>(p_AddedSizeData));
	memcpy(&m_pFrame[m_Size], &l_DataSize, sizeof(l_DataSize));
	m_Size += sizeof(l_DataSize);
}

/******************************************************************************/
void NetworkFrame::addData(const void * p_pData, const size_t p_Size)
{
	memcpy(&m_pFrame[m_Size], p_pData, p_Size);
	m_Size += p_Size;
}

/******************************************************************************/
size_t NetworkFrame::readSize()
{
	if (m_Index >= m_Size)
	{
		werr(LOGDESIGNER, "Frame is empty.");
		return 0U;
	}

	uint32_t l_Size = 0U;
	memcpy(&l_Size, &m_pFrame[m_Index], sizeof(uint32_t));

	l_Size = ntohl(l_Size);

	m_Index += sizeof(uint32_t);

	return l_Size;
}
