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

#include "common.h"
#include "NetworkFrame.h"
#include <arpa/inet.h>
#include <stdio.h>

/******************************************************************************/
NetworkFrame::NetworkFrame() :
		m_pFrame(nullptr), m_Size(0U)
{
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
const uint8_t * NetworkFrame::getFrame() const
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
void NetworkFrame::push(const std::string& p_rStringData)
{
	prepareFrame(p_rStringData.size());
	addData(p_rStringData.c_str(), p_rStringData.size());
}

/******************************************************************************/
void NetworkFrame::push(const std::vector<std::string> & p_rStringVectorData)
{
	for (auto l_It = p_rStringVectorData.begin();
			l_It != p_rStringVectorData.end(); ++l_It)
	{
		prepareFrame(l_It->size());
		addData(l_It->c_str(), l_It->size());
	}
}

/******************************************************************************/
void NetworkFrame::push(const char* p_pAsciiData)
{
	size_t l_Size = strlen(p_pAsciiData);
	prepareFrame(l_Size);
	addData(p_pAsciiData, l_Size);
}

/******************************************************************************/
void NetworkFrame::push(const NetworkFrame & p_rFrame)
{
	size_t l_Size = p_rFrame.getSize();
	prepareFrame(l_Size);
	addData(p_rFrame.getFrame(), l_Size);
}
/******************************************************************************/
void NetworkFrame::prepareFrame(const size_t p_AddedSizeData)
{
	uint_fast32_t l_FrameSize = sizeof(uint32_t) + p_AddedSizeData;

	m_pFrame = static_cast<uint8_t*>(realloc(static_cast<void*>(m_pFrame),
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
