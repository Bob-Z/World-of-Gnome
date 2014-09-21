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
	return file_new(ITEM_TABLE);
}

/**************************************************
 Create a new item based on the specified template
 return the id of the newly created item
 the returned string must be freed by caller
 return NULL if fails
**************************************************/
char * item_create_from_template(const char * template)
{
	char * new_name;
	char * templatename;
	char * newfilename;

	new_name = file_new(ITEM_TABLE);

	templatename = strconcat(base_directory,"/",ITEM_TEMPLATE_TABLE,"/",template,NULL);
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
	char * filename;

	filename = strconcat(ITEM_TABLE,"/",item_id,NULL);
	entry_destroy(filename);
	free(filename);

	return 0;
}

/***********************************************************
 Create a new item resource based on the specified template
 with the specified quantity
 return the id of the newly created item
 the returned string must be freed by caller
 return NULL if fails
***********************************************************/
char * item_resource_new(const char * template, int quantity)
{
	char * new_id;

	new_id = item_create_empty();
	if( new_id == NULL ) {
		return NULL;
	}

	if(!entry_write_string(ITEM_TABLE,new_id,template,ITEM_TEMPLATE, NULL)) {
		entry_destroy(new_id);
		return NULL;
	}

	if(!entry_write_int(ITEM_TABLE,new_id,quantity,ITEM_QUANTITY, NULL)) {
		entry_destroy(new_id);
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
	char * template = NULL;

	entry_read_string(ITEM_TABLE,item_id,&template,ITEM_TEMPLATE, NULL);

	return template;
}

/*****************************
 return the quantity of a resource
 return -1 on error
*****************************/
int resource_get_quantity(const char * item_id)
{
	int quantity;
	char * template;

	if((template=item_is_resource(item_id))==NULL) {
		return 1; /* unique item */
	}
	free(template);

	if(!entry_read_int(ITEM_TABLE,item_id,&quantity,ITEM_QUANTITY, NULL)) {
		return -1;
	}

	return quantity;
}

/*****************************
 set the quantity of a resource
 return -1 on error
*****************************/
int resource_set_quantity(const char * item_id, int quantity)
{
	char * template;

	/* unique item */
	if((template=item_is_resource(item_id))==NULL) {
		return -1;
	}
	free(template);

	if(!entry_write_int(ITEM_TABLE,item_id,quantity,ITEM_QUANTITY, NULL)) {
		return -1;
	}

	return 0;
}

/*****************************
 Return the name of an item
 return NULL on error
*****************************/
char * item_get_name(const char * item_id)
{
	char * template;
	char * name;

	if( (template=item_is_resource(item_id)) != NULL ) {
		if(entry_read_string(ITEM_TEMPLATE_TABLE,template,&name,ITEM_NAME,NULL)) {
			free(template);
			return name;
		}
		free(template);
	} else {
		if(entry_read_string(ITEM_TABLE,item_id,&name,ITEM_NAME,NULL)) {
			return name;
		}
	}

	return NULL;
}
