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

#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include "../common/common.h"
#include <dirent.h>
#include <string.h>

/******************************
 Create an empty new item
 return the id of the newly created item
 the returned string must be freed by caller
 return NULL if fails
*******************************/
gchar * item_create_empty()
{
	return file_new(ITEM_TABLE);
}

/*****************************
 Create a new item based on the specified template
 return the id of the newly created item
 the returned string must be freed by caller
 return NULL if fails
*****************************/
gchar * item_create_from_template(const gchar * template)
{
	gchar * new_name;
	gchar * templatename;
	gchar * newfilename;
	GFile * templatefile;
	GFile * newfile;

	new_name = file_new(ITEM_TABLE);

	templatename = g_strconcat( g_getenv("HOME"),"/", base_directory, "/", ITEM_TEMPLATE_TABLE, "/", template,  NULL);
	templatefile = g_file_new_for_path(templatename);

	newfilename = g_strconcat( g_getenv("HOME"),"/", base_directory, "/", ITEM_TABLE, "/", new_name,  NULL);
	newfile = g_file_new_for_path(newfilename);

	if( g_file_copy(templatefile,newfile, G_FILE_COPY_OVERWRITE,NULL,NULL,NULL,NULL) == FALSE ) {
		g_free(new_name);
		return NULL;
	}

	return new_name;
}

/*****************************
Remove an item file
return -1 if fails
*****************************/
gint item_destroy(const gchar * item_id)
{
	gchar * filename;

	filename = g_strconcat( g_getenv("HOME"),"/", base_directory, "/", ITEM_TABLE, "/", item_id, NULL);

	g_remove(filename);

	return 0;
}

/*****************************
 Create a new item resource based on the specified template
 with the specified quantity
 return the id of the newly created item
 the returned string must be freed by caller
 return NULL if fails
*****************************/
char * item_resource_new(const char * template, int quantity)
{
	char * new_name;

	new_name = item_create_empty();
	if( new_name == NULL ) {
		return NULL;
	}

	if(!write_string(ITEM_TABLE,new_name,template,ITEM_TEMPLATE, NULL)) {
		free(new_name);
		return NULL;
	}

	if(!write_int(ITEM_TABLE,new_name,quantity,ITEM_QUANTITY, NULL)) {
		free(new_name);
		return NULL;
	}

	return new_name;
}

/************************************************
 return template name of resource
 return NULL  if item is unique (i.e. not a resource)
 if not NULL, MUST BE FREED by caller
************************************************/
const char * item_is_resource(const char * item_id)
{
	const char * template = NULL;

	read_string(ITEM_TABLE,item_id,&template,ITEM_TEMPLATE, NULL);

	return template;
}

/*****************************
 return the quantity of an item
 return -1 on error
*****************************/
int item_get_quantity(const char * item_id)
{
	int quantity;
	const char * template;

	if((template=item_is_resource(item_id))==NULL) {
		return 1; /* unique item */
	}

	if(!read_int(ITEM_TABLE,item_id,&quantity,ITEM_QUANTITY, NULL)) {
		return -1;
	}

	return quantity;
}

/*****************************
 set the quantity of a resource item
 return -1 on error
*****************************/
int item_set_quantity(const char * item_id, int quantity)
{
	const char * template;

	if((template=item_is_resource(item_id))==NULL) {
		return -1; /* unique item */
	}

	if(!write_int(ITEM_TABLE,item_id,quantity,ITEM_QUANTITY, NULL)) {
		return -1;
	}

	return 0;
}

/*****************************
 Retrun the name of an item
 return NULL on error
*****************************/
const char * item_get_name(const char * item_id)
{
	const char * template;
	const char * name;

	if( (template=item_is_resource(item_id)) != NULL ) {
		if(read_string(ITEM_TEMPLATE_TABLE,template,&name,ITEM_NAME,NULL)) {
			return name;
		}
	}
	else {
		if(read_string(ITEM_TABLE,item_id,&name,ITEM_NAME,NULL)) {
			return name;
		}
	}

	return NULL;
}
