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

#include "context.h"

void file_lock(const char * filename);
void file_unlock(const char * filename);
void file_update(context_t * context, char * filename);
char * file_new(char * table);
int file_get_contents(const char *filename,char **contents,int *length);
int file_set_contents(const char *filename,const char *contents,int length);
void file_copy(char * src_name, char * dst_name);
int file_create_directory(char * fullname);
int file_delete(const char * table, const char * filename);
