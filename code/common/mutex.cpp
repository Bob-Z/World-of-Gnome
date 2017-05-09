/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013-2017 carabobz@gmail.com

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

#include <SDL2/SDL.h>

SDL_mutex* context_list_mutex = nullptr;
SDL_mutex* npc_mutex = nullptr;
SDL_mutex* attribute_mutex = nullptr;
SDL_mutex* map_mutex = nullptr;
SDL_mutex* file_list_mutex = nullptr;
SDL_mutex* imageDB_mutex = nullptr;
SDL_mutex* entry_mutex = nullptr;
SDL_mutex* character_select_mutex = nullptr;
SDL_mutex* character_create_mutex = nullptr;
SDL_mutex* character_dir_mutex = nullptr;

void common_mutex_init()
{
	context_list_mutex = SDL_CreateMutex();
	npc_mutex = SDL_CreateMutex();
	attribute_mutex = SDL_CreateMutex();
	map_mutex = SDL_CreateMutex();
	file_list_mutex = SDL_CreateMutex();
	imageDB_mutex = SDL_CreateMutex();
	entry_mutex = SDL_CreateMutex();
	character_select_mutex = SDL_CreateMutex();
	character_create_mutex = SDL_CreateMutex();
	character_dir_mutex = SDL_CreateMutex();
}

