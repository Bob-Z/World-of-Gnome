/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2019 carabobz@gmail.com

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

#include "action.h"
#include "common.h"
#include "entry.h"
#include "log.h"
#include "item.h"
#include "syntax.h"
#include "network.h"
#include "network_server.h"
#include "util.h"
#include <dirent.h>
#include <string.h>

/*************************************************************
 Delete the requested item from the character's inventory
 return -1 if fails
 *************************************************************/
int inventory_delete(const char * id, const char * item)
{
	Context * context = context_find(id);
	if (context == nullptr)
	{
		werr(LOGDESIGNER, "Could not find context %s", id);
		return -1;
	}

	if (entry_remove_from_list(CHARACTER_TABLE, context->id, item, CHARACTER_KEY_INVENTORY, nullptr) == RET_OK)
	{
		/* update client */
		network_send_character_file(context);
		return 0;
	}

	network_send_character_file(context);
	return 0;
}

/********************************************************
 Add the requested item to the character's inventory
 return -1 if fails
 ********************************************************/
int inventory_add(const char * ctx_id, const char * item_id)
{
	Context * context = context_find(ctx_id);
	char * mytemplate;
	int index;
	char ** name_list;
	char * current_template;
	int add_count;
	int current_count;

	if (context == nullptr)
	{
		werr(LOGDESIGNER, "Could not find context %s", ctx_id);
		return -1;
	}

	if (item_id == nullptr || item_id[0] == 0)
	{
		return -1;
	}

	// Make sure the CHARACTER_KEY_INVENTORY list exists
	entry_list_create(CHARACTER_TABLE, context->id, CHARACTER_KEY_INVENTORY,
	nullptr);

	mytemplate = item_is_resource(item_id);
	if (mytemplate == nullptr)
	{
		if (entry_add_to_list(CHARACTER_TABLE, context->id, item_id, CHARACTER_KEY_INVENTORY, nullptr) == RET_NOK)
		{
			return -1;
		}
	}
	else
	{
		if (entry_read_int(ITEM_TABLE, item_id, &add_count, ITEM_QUANTITY,
		nullptr) == RET_NOK)
		{
			return -1;
		}
		if (entry_read_list(CHARACTER_TABLE, context->id, &name_list, CHARACTER_KEY_INVENTORY, nullptr) == RET_NOK)
		{
			return -1;
		}

		index = 0;
		while (name_list[index] != nullptr)
		{
			if (entry_read_string(ITEM_TABLE, name_list[index], &current_template, ITEM_TEMPLATE, nullptr) == RET_OK)
			{
				if (strcmp(mytemplate, current_template) == 0)
				{
					if (entry_read_int(ITEM_TABLE, name_list[index], &current_count, ITEM_QUANTITY, nullptr) == RET_OK)
					{
						free(current_template);
						free(mytemplate);
						add_count += current_count;
						resource_set_quantity(context, name_list[index], add_count);
						item_destroy(item_id);
						network_send_table_file(context, ITEM_TABLE, name_list[index]);
						return 0;
					}
				}
				free(current_template);
			}
			index++;
		}
		free(mytemplate);

		/* First time we add this type of resource to inventory */
		if (name_list[index] == nullptr)
		{
			if (entry_add_to_list(CHARACTER_TABLE, context->id, item_id, CHARACTER_KEY_INVENTORY, nullptr) == RET_NOK)
			{
				return -1;
			}
		}

		deep_free(name_list);
	}

	network_send_character_file(context);
	return 0;
}

/***************************************************************************
 return an item ID of an item in inventory with specified name
 the returned string must be freed
 ***************************************************************************/
char * inventory_get_by_name(const char * id, const char * item_name)
{
	int index;
	char ** name_list;
	char * name;
	char * res;

	Context * context = context_find(id);
	if (context == nullptr)
	{
		werr(LOGDESIGNER, "Could not find context %s", id);
		return nullptr;
	}

	if (entry_read_list(CHARACTER_TABLE, context->id, &name_list, CHARACTER_KEY_INVENTORY, nullptr) == RET_NOK)
	{
		return nullptr;
	}

	index = 0;
	while (name_list[index] != nullptr)
	{
		if ((name = item_get_name(name_list[index])) != nullptr)
		{
			if (strcmp(item_name, name) == 0)
			{
				res = strdup(name_list[index]);
				deep_free(name_list);
				free(name);
				return res;
			}
			free(name);
		}
		index++;
	}

	deep_free(name_list);
	return nullptr;
}
