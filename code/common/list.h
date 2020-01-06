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

#ifndef LIST_H
#define LIST_H

#define HASH_TABLE_SIZE 65536

typedef struct list_tag {
	const char * key;
	void * data;
	unsigned long hash;
	struct list_tag * next;
} list_entry_t;

typedef list_entry_t * list_t;

void * list_find(list_t * list, const char * key);
void list_update(list_t ** list, const char *key, void * data);

#endif
