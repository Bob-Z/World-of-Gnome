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

#include "Context.h"
#include "entry.h"
#include "file.h"
#include "network.h"
#include "syntax.h"
#include <stdlib.h>
#include <string>
#include <utility>

/********************************************
 Create an empty new item
 return the id of the newly created item
 the returned string must be freed by caller
 return nullptr if fails
 ********************************************/
std::pair<bool, std::string> item_create_empty()
{
	return file_new(std::string(ITEM_TABLE.c_str()));
}

/**************************************************
 Create a new item based on the specified template
 return the id of the newly created item
 the returned string must be freed by caller
 **************************************************/
std::pair<bool, std::string> item_create_from_template(
		const std::string &my_template)
{
	const std::pair<bool, std::string> new_name = file_new(ITEM_TABLE.c_str());

	if (new_name.first == true)
	{
		if (file_copy(ITEM_TEMPLATE_TABLE.c_str(), my_template.c_str(),
				ITEM_TABLE.c_str(), new_name.second.c_str()) == false)
		{
			file_delete(ITEM_TABLE.c_str(), new_name.second);
			return std::pair<bool, std::string>
			{ false, "" };
		}
	}

	return new_name;
}

/*****************************
 Remove an item file
 return false on error
 *****************************/
int item_destroy(const std::string &item_id)
{
	return entry_destroy(ITEM_TABLE.c_str(), item_id.c_str());
}

/***********************************************************
 Create a new resource
 with the specified quantity
 return the id of the newly created resource
 ***********************************************************/
std::pair<bool, std::string> resource_new(const std::string &my_template,
		int quantity)
{
	const std::pair<bool, std::string> new_id = item_create_empty();

	if (new_id.first == false)
	{
		return new_id;
	}

	if (entry_write_string(ITEM_TABLE.c_str(), new_id.second.c_str(),
			my_template.c_str(), ITEM_TEMPLATE, nullptr) == false)
	{
		entry_destroy(ITEM_TABLE.c_str(), new_id.second.c_str());
		return std::pair<bool, std::string>
		{ false, "" };
	}

	if (entry_write_int(ITEM_TABLE.c_str(), new_id.second.c_str(), quantity,
			ITEM_QUANTITY, nullptr) == false)
	{
		entry_destroy(ITEM_TABLE.c_str(), new_id.second.c_str());
		return std::pair<bool, std::string>
		{ false, "" };
	}

	return new_id;
}

/*****************************************************
 return template name of resource
 Returned string MUST BE FREED
 return nullptr  if item is unique (i.e. not a resource)
 *****************************************************/
char* item_is_resource(const std::string &inventory_id)
{
	char *my_template = nullptr;

	entry_read_string(ITEM_TABLE.c_str(), inventory_id.c_str(), &my_template,
			ITEM_TEMPLATE, nullptr);

	return my_template;
}

/*****************************
 return the quantity of a resource
 return -1 on error
 *****************************/
int resource_get_quantity(const std::string &item_id)
{
	int quantity;
	char *my_template;

	if ((my_template = item_is_resource(item_id)) == nullptr)
	{
		return 1; /* unique item */
	}
	free(my_template);

	if (entry_read_int(ITEM_TABLE.c_str(), item_id.c_str(), &quantity,
			ITEM_QUANTITY, nullptr) == false)
	{
		return -1;
	}

	return quantity;
}

/*****************************
 set the quantity of a resource
 return -1 on error
 *****************************/
int resource_set_quantity(Context *context, const std::string &item_id,
		int quantity)
{
	char *my_template;

	// unique item
	if ((my_template = item_is_resource(item_id)) == nullptr)
	{
		return -1;
	}
	free(my_template);

	if (entry_write_int(ITEM_TABLE.c_str(), item_id.c_str(), quantity,
			ITEM_QUANTITY, nullptr) == false)
	{
		return -1;
	}

	network_send_table_file(context, ITEM_TABLE.c_str(), item_id.c_str());

	return 0;
}

/*****************************
 Return the name of an item
 return nullptr on error
 *****************************/
char* item_get_name(const std::string &item_id)
{
	char *my_template;
	char *name;

	if ((my_template = item_is_resource(item_id)) != nullptr)
	{
		if (entry_read_string(ITEM_TEMPLATE_TABLE.c_str(), my_template, &name,
				ITEM_NAME.c_str(), nullptr) == true)
		{
			free(my_template);
			return name;
		}
		free(my_template);
	}
	else
	{
		if (entry_read_string(ITEM_TABLE.c_str(), item_id.c_str(), &name,
				ITEM_NAME.c_str(), nullptr) == true)
		{
			return name;
		}
	}

	return nullptr;
}
