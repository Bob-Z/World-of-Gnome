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

#include "list.h"
#include <stdlib.h>
#include <cstring>

/****************************************
 Hash calculation
 *****************************************/
static unsigned long calc_hash(const char * str)
{
	unsigned long hash = 5381;
	int c;

	while ((c = *str++))
	{
		hash = hash * 33 ^ c; // hash(i) = hash(i - 1) * 33 ^ str[i];
		//hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}

	return hash;
}

/****************************************
 Return the list_t associated with the given key
 Return nullptr if key does not match any list entry.
 *****************************************/
static list_entry_t * search_entry(list_t * list, const char * key)
{
	list_entry_t * current_entry;
	unsigned long hash;
	int index;

	if (list == nullptr)
	{
		return nullptr;
	}

	hash = calc_hash(key);
	index = hash % HASH_TABLE_SIZE;

	current_entry = list[index];

	while (current_entry)
	{
		if (current_entry->hash == hash)
		{
			return current_entry;
		}
		current_entry = current_entry->next;
	}
	return nullptr;
}

/****************************************
 Return the data pointer associated with the given key
 Return nullptr if key does not match any list entry.
 *****************************************/
void * list_find(list_t * list, const char * key)
{
	list_entry_t * current_entry;

	if (list == nullptr)
	{
		return nullptr;
	}

	current_entry = search_entry(list, key);

	if (current_entry)
	{
		return current_entry->data;
	}

	return nullptr;
}

/****************************************
 Add or update an entry into the given list.
 key and data pointers are copied in the list. The user should not free them.
 *****************************************/
void list_update(list_t ** list, const char *key, void * data)
{
	list_entry_t * entry;
	list_entry_t * new_entry;
	int index;

	entry = search_entry(*list, key);

	/* The key already exists, update the entry */
	if (entry)
	{
		entry->data = data;
		return;
	}

	/* The key doesn't exists; create it */
	new_entry = (list_entry_t*) malloc(sizeof(list_entry_t));
	new_entry->key = strdup(key);
	new_entry->data = data;
	new_entry->hash = calc_hash(new_entry->key);
	new_entry->next = nullptr;

	index = new_entry->hash % HASH_TABLE_SIZE;

	/* list does not exist */
	if (*list == nullptr)
	{
		*list = (list_entry_t**) malloc( HASH_TABLE_SIZE * sizeof(list_entry_t *));
		memset(*list, 0, HASH_TABLE_SIZE * sizeof(list_entry_t *));
		(*list)[index] = new_entry;
		return;
	}

	/* list exists and the hash table index is already occupied */
	if ((*list)[index] != nullptr)
	{
		entry = (*list)[index];
		while (entry->next != nullptr)
		{
			entry = entry->next;
		}

		entry->next = new_entry;
	}
	/* list exists and hash table index is free */
	else
	{
		(*list)[index] = new_entry;
	}

	return;
}
