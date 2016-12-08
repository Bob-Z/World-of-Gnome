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

static list_t * font_list = NULL;

/****************************************
*****************************************/
TTF_Font * font_get(context_t* ctx,const char * filename, int size)
{
	TTF_Font * font;
	char * fullname;

	file_lock(filename);

	font = (TTF_Font*)list_find(font_list,filename);

	if( font ) {
		file_unlock(filename);
		return font;
	}

	fullname = strconcat(base_directory,"/",filename,NULL);
	font = TTF_OpenFont(fullname, size);
	free(fullname);
	if( font ) {
		list_update(&font_list,filename,font);
		file_unlock(filename);
		return font;
	}

	file_update(ctx,filename);

	file_unlock(filename);

	return NULL;
}

