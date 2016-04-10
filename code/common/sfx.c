/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2016 carabobz@gmail.com

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

#include "common.h"
#include <SDL2/SDL_mixer.h>

static list_t * sfx_list = NULL;

/****************************************
*****************************************/
void sfx_play(context_t* ctx,const char * filename, int restart)
{
	Mix_Music * music;
	char * table_filename;
	char * fullname;

	table_filename = strconcat(SFX_TABLE,"/",filename,NULL);

	file_lock(table_filename);

	music = list_find(sfx_list,table_filename);

	if( music ) {
		if( restart ) {
			Mix_PlayMusic(music, -1);
		}
		file_unlock(table_filename);
		return;
	}

	fullname = strconcat(base_directory,"/",table_filename,NULL);
	music = Mix_LoadMUS(fullname);
	free(fullname);

	if( music ) {
		Mix_PlayMusic(music, -1);
		list_update(&sfx_list,table_filename,music);
		file_unlock(table_filename);
		return;
	}

	file_update(ctx,table_filename);

	file_unlock(table_filename);

	return;
}

/****************************************
*****************************************/
void sfx_stop(context_t* ctx,const char * filename)
{
	Mix_Music * music;
	char * table_filename;

	table_filename = strconcat(SFX_TABLE,"/",filename,NULL);

	file_lock(table_filename);

	music = list_find(sfx_list,table_filename);

	file_unlock(table_filename);

	if( music ) {
		Mix_FreeMusic(music);
		list_update(&sfx_list,table_filename,NULL);
		return;
	}

	return;
}

