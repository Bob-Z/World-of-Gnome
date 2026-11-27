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

#include "action.h"
#include "entry.h"
#include "item.h"
#include "log.h"
#include "network_server.h"
#include "network.h"
#include "syntax.h"
#include "util.h"
#include <dirent.h>
#include <string.h>

/*************************************************************
 Delete the requested item from the character's inventory
 return -1 if fails
 *************************************************************/
int inventory_delete(const char *id, const char *item)
{
	Context *context = context_find(id);
	if (context == nullptr)
	{
		werr(LOGDESIGNER, "Could not find context %s", id);
		return -1;
	}

	if (entry_remove_from_list(CHARACTER_TABLE.c_str(),
			context->getId().c_str(), item, { CHARACTER_KEY_INVENTORY.c_str() }) == true)
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
int inventory_add(const char *ctx_id, const char *item_id)
{
	Context *context = context_find(ctx_id);
	std::string mytemplate;
	std::vector<std::string> name_list;
	std::string current_template;
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

	// Make sure the CHARACTER_KEY_INVENTORY.c_str() list exists
	entry_list_create(CHARACTER_TABLE.c_str(), context->getId().c_str(), { CHARACTER_KEY_INVENTORY.c_str() });

	mytemplate = item_is_resource(std::string(item_id));
	if (mytemplate.empty())
	{
		if (entry_add_to_list(CHARACTER_TABLE.c_str(), context->getId().c_str(),
				item_id, { CHARACTER_KEY_INVENTORY.c_str() }) == false)
		{
			return -1;
		}
	}
	else
	{
		if (entry_read_int(ITEM_TABLE.c_str(), item_id, &add_count, { ITEM_QUANTITY.c_str() }) == false)
		{
			return -1;
		}
		if (entry_read_list(CHARACTER_TABLE.c_str(), context->getId().c_str(),
				name_list, { CHARACTER_KEY_INVENTORY.c_str() }) == false)
		{
			return -1;
		}

		for (const auto &name : name_list)
		{
			if (entry_read_string(ITEM_TABLE.c_str(), name.c_str(),
					current_template, { ITEM_TEMPLATE.c_str() }) == true)
			{
				if (mytemplate == current_template)
				{
					if (entry_read_int(ITEM_TABLE.c_str(), name.c_str(),
							&current_count, { ITEM_QUANTITY.c_str() }) == true)
					{
						add_count += current_count;
						resource_set_quantity(context, name, add_count);
						item_destroy(std::string(item_id));
						network_send_table_file(context, ITEM_TABLE.c_str(),
								name.c_str());
						return 0;
					}
				}
			}
		}

		// No matching template in the inventory: add it
		{
			if (entry_add_to_list(CHARACTER_TABLE.c_str(),
					context->getId().c_str(), item_id, { CHARACTER_KEY_INVENTORY.c_str() }) == false)
			{
				return -1;
			}
		}
	}

	network_send_character_file(context);
	return 0;
}

/***************************************************************************
 return an item ID of an item in inventory with specified name
 the returned string must be freed
 ***************************************************************************/
std::string inventory_get_by_name(const char *id, const char *item_name)
{
	std::vector<std::string> name_list;

	Context *context = context_find(id);
	if (context == nullptr)
	{
		werr(LOGDESIGNER, "Could not find context %s", id);
		return std::string();
	}

	if (entry_read_list(CHARACTER_TABLE.c_str(), context->getId().c_str(),
			name_list, { CHARACTER_KEY_INVENTORY.c_str() }) == false)
	{
		return std::string();
	}

	for (const auto &itemId : name_list)
	{
		std::string name = item_get_name(itemId.c_str());
		if (name.empty() == false)
		{
			if (item_name == name)
			{
				return itemId;
			}
		}
	}
	return std::string();
}
