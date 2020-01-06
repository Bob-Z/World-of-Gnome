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

#ifndef COMMON_CONTEXTLIST_H_
#define COMMON_CONTEXTLIST_H_

#include "Context.h"
#include <map>
#include <SDL2/SDL_mutex.h>
#include <string>
#include <tuple>

class ContextGetter;

class ContextList
{
	friend ContextGetter;

public:
	ContextList();
	virtual ~ContextList();

	void add(std::pair<std::string, Context> & to_be_added);

private:
	SDL_mutex* getMutex() const;
	Context & get(const std::string & contextId);

	std::map<std::string, Context> m_list;
	SDL_mutex* m_mutex;
};

#endif /* COMMON_CONTEXTLIST_H_ */
