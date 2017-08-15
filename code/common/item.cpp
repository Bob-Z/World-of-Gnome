/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013-2017 carabobz@gmail.com

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

#include "../common/common.h"
#include <dirent.h>
#include <string.h>

/********************************************
 Create an empty new item
 return the id of the newly created item
 the returned string must be freed by caller
 return nullptr if fails
********************************************/
char * item_create_empty()
{
	return file_new(ITEM_TABLE,nullptr);
}

/**************************************************
 Create a new item based on the specified template
 return the id of the newly created item
 the returned string must be freed by caller
 return nullptr if fails
**************************************************/
char * item_create_from_template(const char * my_template)
{
	char * new_name;

	new_name = file_new(ITEM_TABLE,nullptr);
	if( file_copy(ITEM_TEMPLATE_TABLE,my_template,ITEM_TABLE,new_name) == false ) {
		file_delete(ITEM_TABLE, new_name);
                return nullptr;
        }

	return new_name;
}

/*****************************
Remove an item file
return RET_NOK on error
*****************************/
ret_code_t item_destroy(const char * item_id)
{
	return entry_destroy(ITEM_TABLE,item_id);
}

/***********************************************************
 Create a new resource
 with the specified quantity
 return the id of the newly created resource
 the returned string must be freed by caller
 return nullptr if fails
***********************************************************/
char * resource_new(const char * my_template, int quantity)
{
	char * new_id;

	new_id = item_create_empty();
	if( new_id == nullptr ) {
		return nullptr;
	}

	if(entry_write_string(ITEM_TABLE,new_id,my_template,ITEM_TEMPLATE, nullptr) == RET_NOK ) {
		entry_destroy(ITEM_TABLE,new_id);
		return nullptr;
	}

	if(entry_write_int(ITEM_TABLE,new_id,quantity,ITEM_QUANTITY, nullptr) == RET_NOK ) {
		entry_destroy(ITEM_TABLE,new_id);
		return nullptr;
	}

	return new_id;
}

/*****************************************************
 return template name of resource
 Returned string MUST BE FREED
 return nullptr  if item is unique (i.e. not a resource)
 *****************************************************/
char * item_is_resource(const char * item_id)
{
	char * my_template = nullptr;

	entry_read_string(ITEM_TABLE,item_id,&my_template,ITEM_TEMPLATE, nullptr);

	return my_template;
}

/*****************************
 return the quantity of a resource
 return -1 on error
*****************************/
int resource_get_quantity(const char * item_id)
{
	int quantity;
	char * my_template;

	if((my_template=item_is_resource(item_id))==nullptr) {
		return 1; /* unique item */
	}
	free(my_template);

	if(entry_read_int(ITEM_TABLE,item_id,&quantity,ITEM_QUANTITY, nullptr) == RET_NOK ) {
		return -1;
	}

	return quantity;
}

/*****************************
 set the quantity of a resource
 return -1 on error
*****************************/
int resource_set_quantity(context_t * context, const char * item_id, int quantity)
{
	char * my_template;

	/* unique item */
	if((my_template=item_is_resource(item_id))==nullptr) {
		return -1;
	}
	free(my_template);

	if(entry_write_int(ITEM_TABLE,item_id,quantity,ITEM_QUANTITY, nullptr) == RET_NOK ) {
		return -1;
	}

	network_send_table_file(context,ITEM_TABLE,item_id);

	return 0;
}

/*****************************
 Return the name of an item
 return nullptr on error
*****************************/
char * item_get_name(const char * item_id)
{
	char * my_template;
	char * name;

	if( (my_template=item_is_resource(item_id)) != nullptr ) {
		if(entry_read_string(ITEM_TEMPLATE_TABLE,my_template,&name,ITEM_NAME,nullptr) == RET_OK ) {
			free(my_template);
			return name;
		}
		free(my_template);
	} else {
		if(entry_read_string(ITEM_TABLE,item_id,&name,ITEM_NAME,nullptr) == RET_OK ) {
			return name;
		}
	}

	return nullptr;
}
