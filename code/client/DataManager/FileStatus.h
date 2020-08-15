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

#ifndef CLIENT_DATAMANAGER_FILESTATUS_H_
#define CLIENT_DATAMANAGER_FILESTATUS_H_

#include <SDL2/SDL.h>

class FileStatus
{
public:
	FileStatus();
	virtual ~FileStatus();

	Uint32 getTimeStamp() const;
	void setTimeStamp(Uint32 timeStamp);

private:
	Uint32 m_timeStamp;
};

#endif /* CLIENT_DATAMANAGER_FILESTATUS_H_ */
