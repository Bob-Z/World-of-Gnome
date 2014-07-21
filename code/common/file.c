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

#include "file.h"
#include "list.h"
#include "network.h"

/* TODO: push this in const.h */
#define _FILE_REQUEST_TIMEOUT 1000
list_t * file_list = NULL;

/****************************************
filename is "table/dir/file"
*****************************************/
void file_lock(char * filename)
{
	file_t * file_data;
	
	file_data = list_find(file_list,filename);
	if( file_data == NULL ) {
		file_data = malloc(sizeof(file_t));
		file_data->timestamp = 0;
		file_data->mutex = SDL_CreateMutex();
		
		file_list = list_update(file_list,filename,file_data);
	}
	
	SDL_LockMutex(file_data->mutex);
}

/****************************************
filename is "table/dir/file"
*****************************************/
void file_unlock(char * filename)
{
	file_t * file_data;
	
	file_data = list_find(file_list,filename);
	if( file_data == NULL ) {
		return;
	}
	
	SDL_UnlockMutex(file_data->mutex);
}

/****************************************
filename is "table/dir/file"
*****************************************/
void file_update(context_t * context, char * filename)
{
	Uint32 current_time = SDL_GetTicks();
	file_t * file_data;

	file_data = list_find(file_list,filename);

	if( file_data->timestamp + _FILE_REQUEST_TIMEOUT > current_time ) {
		return;
	}
	
	network_send_req_file(context, filename);
}
