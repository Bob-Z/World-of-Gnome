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

#include "common.h"

static list_t * list_search(list_t * list, const char * key)
{
	list_t * current_list = list;
	int i;
	
	while(current_list) {
		i = 0;
		while(current_list->key[i] != 0 && key[i] != 0) {
			if( current_list->key[i] != key[i] ) {
				if( current_list->next == NULL ) {
					return NULL;
				}
				current_list = current_list->next;
				break;
			}
			i++;
		}

		/* Key is found */
		if( current_list->key[i] == 0 && key[i] == 0 ) {
			return current_list;
		}
		
		current_list = current_list->next;
	}
	return NULL;
}

/****************************************
Return the pointer associated with the given key
Return NULL if key does not match any list entry.
*****************************************/
void * list_find(list_t * list, const char * key)
{
	list_t * current_list;
	
	current_list = list_search(list,key);

	if( current_list ) {
		return current_list->data;
	}
	
	return NULL;
}

/****************************************
Add or update an entry into the given list.
key and data pointers are copied in the list. The user should not free them.
Return the head of the list.
*****************************************/
list_t * list_update(list_t * list, const char *key, void * data)
{
	list_t * current_list = list;
	list_t * new_list;
	
	current_list = list_search(list,key);
	
	/* The key already exists, update the entry */
	if( current_list ) {
		current_list->data = data;
		return list;
	}

	/* The key doesn't exists; create it */
	new_list = malloc(sizeof(list_t));
	new_list->key = strdup(key);
	new_list->data = data;
	new_list->next = NULL;
	
	if( list == NULL) {
		return new_list;
	}

	current_list = list;	
	while(current_list->next != NULL) {
		current_list = current_list->next;
	}

	current_list->next = new_list;
	
	return list;
}
