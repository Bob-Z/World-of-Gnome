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

#include "ContextList.h"

#include "SdlLocking.h"

/*****************************************************************************/
ContextList::ContextList() :
		m_list(), m_mutex()
{
	m_mutex = SDL_CreateMutex();
}

/*****************************************************************************/
ContextList::~ContextList()
{
	if (m_mutex != nullptr)
	{
		SDL_DestroyMutex(m_mutex);
	}
}

/*****************************************************************************/
void ContextList::add(std::pair<std::string, Context> & to_be_added)
{
	SdlLocking lock(m_mutex);

	m_list.insert(to_be_added);
}

/*****************************************************************************/
Context & ContextList::get(const std::string & contextId)
{
	SdlLocking lock(m_mutex);

	auto iter = m_list.find(contextId);

	if (iter == m_list.end())
	{
		throw std::runtime_error("Cannot find context with ID: " + contextId);
	}

	return iter->second;
}

/*****************************************************************************/
SDL_mutex* ContextList::getMutex() const
{
	return m_mutex;
}
