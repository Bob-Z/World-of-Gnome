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

#include "ContextContainer.h"

#include "Context.h"

/*****************************************************************************/
ContextContainer::ContextContainer() :
		m_list()
{
}

/*****************************************************************************/
void ContextContainer::add(const std::string & id)
{
	m_list.insert(std::make_pair(id, std::make_shared<Context>()));
}

/*****************************************************************************/
void ContextContainer::remove(const std::string & id)
{
	m_list.erase(id);
}

/*****************************************************************************/
std::shared_ptr<Context> ContextContainer::get(const std::string & id) const
{
	auto iter = m_list.find(id);

	if (iter == m_list.end())
	{
		throw std::runtime_error("Context " + id + " not found");
	}

	return iter->second;
}

/*****************************************************************************/
std::map<std::string, std::shared_ptr<Context>>::const_iterator ContextContainer::begin() const
{
	return m_list.cbegin();
}

/*****************************************************************************/
std::map<std::string, std::shared_ptr<Context>>::const_iterator ContextContainer::end() const
{
	return m_list.cend();
}
