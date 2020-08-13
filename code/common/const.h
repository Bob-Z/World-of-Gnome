/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2020 carabobz@gmail.com

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

#ifndef COMMON_CONST_H
#define COMMON_CONST_H

#include <SDL_stdinc.h>
#include <string>

static constexpr Uint16 PORT = 42520U;
static const std::string APP_NAME("wog");
static const std::string TITLE_NAME("Worlds of Gnome");
#define SMALL_BUF (1024)

// delay before two consecutive request of the same file (in milliseconds)
#define FILE_REQUEST_TIMEOUT (5000)

#endif
