/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013-2016 carabobz@gmail.com

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
 return NULL if fails
********************************************/
char * item_create_empty()
{
	return file_new(ITEM_TABLE,NULL);
}

/**************************************************
 Create a new item based on the specified template
 return the id of the newly created item
 the returned string must be freed by caller
 return NULL if fails
**************************************************/
char * item_create_from_template(const char * my_template)
{
	char * new_name;
	char * templatename;
	char * newfilename;

	new_name = file_new(ITEM_TABLE,NULL);

	templatename = strconcat(base_directory,"/",ITEM_TEMPLATE_TABLE,"/",my_template,NULL);
	newfilename = strconcat(base_directory,"/",ITEM_TABLE,"/",new_name,NULL);

	file_copy(templatename,newfilename);
	free(newfilename);
	free(templatename);

	return new_name;
}

/*****************************
Remove an item file
return -1 if fails
*****************************/
int item_destroy(const char * item_id)
{
	return entry_destroy(ITEM_TABLE,item_id);
}

/***********************************************************
 Create a new resource
 with the specified quantity
 return the id of the newly created resource
 the returned string must be freed by caller
 return NULL if fails
***********************************************************/
char * resource_new(const char * my_template, int quantity)
{
	char * new_id;

	new_id = item_create_empty();
	if( new_id == NULL ) {
		return NULL;
	}

	if(!entry_write_string(ITEM_TABLE,new_id,my_template,ITEM_TEMPLATE, NULL)) {
		entry_destroy(ITEM_TABLE,new_id);
		return NULL;
	}

	if(!entry_write_int(ITEM_TABLE,new_id,quantity,ITEM_QUANTITY, NULL)) {
		entry_destroy(ITEM_TABLE,new_id);
		return NULL;
	}

	return new_id;
}

/*****************************************************
 return template name of resource
 Returned string MUST BE FREED
 return NULL  if item is unique (i.e. not a resource)
 *****************************************************/
char * item_is_resource(const char * item_id)
{
	char * my_template = NULL;

	entry_read_string(ITEM_TABLE,item_id,&my_template,ITEM_TEMPLATE, NULL);

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

	if((my_template=item_is_resource(item_id))==NULL) {
		return 1; /* unique item */
	}
	free(my_template);

	if(!entry_read_int(ITEM_TABLE,item_id,&quantity,ITEM_QUANTITY, NULL)) {
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
	if((my_template=item_is_resource(item_id))==NULL) {
		return -1;
	}
	free(my_template);

	if(!entry_write_int(ITEM_TABLE,item_id,quantity,ITEM_QUANTITY, NULL)) {
		return -1;
	}

	network_send_table_file(context,ITEM_TABLE,item_id);

	return 0;
}

/*****************************
 Return the name of an item
 return NULL on error
*****************************/
char * item_get_name(const char * item_id)
{
	char * my_template;
	char * name;

	if( (my_template=item_is_resource(item_id)) != NULL ) {
		if(entry_read_string(ITEM_TEMPLATE_TABLE,my_template,&name,ITEM_NAME,NULL)) {
			free(my_template);
			return name;
		}
		free(my_template);
	} else {
		if(entry_read_string(ITEM_TABLE,item_id,&name,ITEM_NAME,NULL)) {
			return name;
		}
	}

	return NULL;
}
