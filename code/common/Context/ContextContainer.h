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

#ifndef COMMON_CONTEXT_CONTEXTCONTAINER_H_
#define COMMON_CONTEXT_CONTEXTCONTAINER_H_

#include "Context.h"
#include <map>
#include <memory>
#include <string>

class Context;

class ContextContainer
{
public:
	ContextContainer();
	virtual ~ContextContainer() = default;

	std::shared_ptr<Context> add(const std::string & id);
	void remove(const std::string & id);

	std::shared_ptr<Context> get(const std::string & id) const;

	std::map<std::string, std::shared_ptr<Context>>::const_iterator begin() const;
	std::map<std::string, std::shared_ptr<Context>>::const_iterator end() const;

private:
	std::map<std::string, std::shared_ptr<Context>> m_list;
};

#endif /* COMMON_CONTEXT_CONTEXTCONTAINER_H_ */
