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

#ifndef ENTRY_H
#define ENTRY_H

void entry_remove(char * filename);

int read_int(const char * table, const char * file, int * res, ...);
int entry_read_string(const char * table, const char * file, char ** res, ...);
int read_list_index(const char * table, const char * file, const char ** res,int index, ...);
int read_list(const char * table, const char * file, char *** res, ...);

int write_int(const char * table, const char * file, int data, ...);
int write_string(const char * table, const char * file,const char * data, ...);
int write_list_index(const char * table, const char * file, const char * data,int index, ...);
int write_list(const char * table, const char * file, char ** data, ...);

char * get_unused_list_entry(const char * table, const char * file, ...);
char * get_unused_group(const char * table, const char * file, ...);
int get_group_list(const char * table, const char * file, char *** res, ...);

int add_to_list(const char * table, const char * file, const char * to_be_added, ...);
int remove_group(const char * table, const char * file, const char * group, ...);

int remove_from_list(const char * table, const char * file, const char * to_be_removed, ...);
int list_create(const char * table, const char * file, ...);
int group_create(const char * table, const char * file, ...);
char * copy_group(const char * src_table, const char * src_file, const char * dst_table, const char * dst_file, const char * group_name, ...);
int entry_update(char * data);
int entry_destroy(const char * id);
#endif
