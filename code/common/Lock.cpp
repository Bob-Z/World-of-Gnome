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

#include "Lock.h"

Lock::Lock() :
		m_lock(SDL_CreateMutex())
{
}

Lock::~Lock()
{
	SDL_DestroyMutex(m_lock);
}

int Lock::trylock()
{
	return SDL_TryLockMutex(m_lock);
}

void Lock::lock()
{
	SDL_LockMutex(m_lock);
}

void Lock::unlock()
{
	SDL_UnlockMutex(m_lock);
}

SDL_mutex* Lock::getLock() const
{
	return m_lock;
}
