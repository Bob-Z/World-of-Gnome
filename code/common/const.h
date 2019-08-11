/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013-2019 carabobz@gmail.com

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

#ifndef CONST_H
#define CONST_H

#define MAX_CLIENT 10
#define PORT 42520
#define APP_NAME "wog"
#define TITLE_NAME "WOG"
#define SMALL_BUF 1024
#define BIG_BUF 10240
/* delay before two consecutive request of the same file (in milliseconds) */
#define FILE_REQUEST_TIMEOUT (5000)
/* Max number of parameter for an action command */
#define MAX_PARAMETER   32

#endif
