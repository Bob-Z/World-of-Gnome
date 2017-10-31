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
#include <stdio.h>

/******************************************************************************/
NetworkFrame::NetworkFrame() :
		m_pFrame(nullptr)
{
	m_pFrame = static_cast<char*>(malloc(1));
	m_pFrame[0] = '\0';
}

/******************************************************************************/
NetworkFrame::NetworkFrame(char * p_pFrame) :
		m_pFrame(nullptr)
{
	if (p_pFrame != nullptr)
	{
		m_pFrame = strdup(p_pFrame);
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
const char * NetworkFrame::getFrame()
{
	return m_pFrame;
}

/******************************************************************************/
void NetworkFrame::add(int p_IntData)
{
	char l_pBuff[SMALL_BUF];

	snprintf(l_pBuff, SMALL_BUF, "%d", p_IntData);

	char * l_pFrame = nullptr;
	if (strlen(m_pFrame) == 0)
	{
		l_pFrame = strconcat(m_pFrame, l_pBuff, nullptr);
	}
	else
	{
		l_pFrame = strconcat(m_pFrame, NETWORK_DELIMITER, l_pBuff, nullptr);
	}

	free(m_pFrame);
	m_pFrame = l_pFrame;
}

/******************************************************************************/
void NetworkFrame::add(const std::string& p_rStringData)
{
	char * l_pFrame = nullptr;
	if (strlen(m_pFrame) == 0)
	{
		l_pFrame = strconcat(m_pFrame, p_rStringData.c_str(), nullptr);
	}
	else
	{
		l_pFrame = strconcat(m_pFrame, NETWORK_DELIMITER, p_rStringData.c_str(),
				nullptr);
	}

	free(m_pFrame);
	m_pFrame = l_pFrame;
}

/******************************************************************************/
void NetworkFrame::add(const std::vector<std::string> & p_rStringVectorData)
{
	char * l_pFrame = nullptr;
	for (auto l_It = p_rStringVectorData.begin();
			l_It != p_rStringVectorData.end(); ++l_It)
	{
		if (strlen(m_pFrame) == 0)
		{
			l_pFrame = strconcat(m_pFrame, l_It->c_str(), nullptr);
		}
		else
		{
			l_pFrame = strconcat(m_pFrame, NETWORK_DELIMITER, l_It->c_str(),
					nullptr);
		}

		free(m_pFrame);
		m_pFrame = l_pFrame;
	}
}

/******************************************************************************/
void NetworkFrame::add(const char* p_rAsciiData)
{
	char * l_pFrame = nullptr;
	if (strlen(m_pFrame) == 0)
	{
		l_pFrame = strconcat(m_pFrame, p_rAsciiData, nullptr);
	}
	else
	{
		l_pFrame = strconcat(m_pFrame, NETWORK_DELIMITER, p_rAsciiData,
				nullptr);
	}

	free(m_pFrame);
	m_pFrame = l_pFrame;
}
