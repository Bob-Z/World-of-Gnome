/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013-2014 carabobz@gmail.com

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

#ifndef MUTEX_C
#define MUTEX_C

#include <SDL2/SDL.h>

extern SDL_mutex* context_list_mutex;
extern SDL_mutex* npc_mutex;
extern SDL_mutex* attribute_mutex;
extern SDL_mutex* map_mutex;
extern SDL_mutex* npc_start_mutex;
extern SDL_mutex* file_list_mutex;
extern SDL_mutex* imageDB_mutex;
extern SDL_mutex* entry_mutex;
extern SDL_mutex* character_select_mutex;

void common_mutex_init();

#endif

