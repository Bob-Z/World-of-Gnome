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

#ifndef COMMON_LOCK_H_
#define COMMON_LOCK_H_

#include <SDL_mutex.h>

class Lock
{
public:
	Lock();
	virtual ~Lock();

	int trylock();
	void lock();
	void unlock();

	SDL_mutex* getLock() const;

private:
	SDL_mutex * m_lock;
};

#endif /* COMMON_LOCK_H_ */
