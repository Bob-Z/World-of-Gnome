/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013 carabobz@gmail.com

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
#include "action.h"

/*****************************/
/* Create an empty new item */
/* return the id of the newly created item */
/* the returned string must be freed by caller */
/* return NULL if fails */
gchar * item_create_empty()
{
	return file_new(ITEM_TABLE);
}

/*****************************/
/* Create a new item based on the specified template */
/* return the id of the newly created item */
/* the returned string must be freed by caller */
/* return NULL if fails */
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

/* Remove an item file */
/* return -1 if fails */
gint item_destroy(const gchar * item_id)
{
        gchar * filename;

        filename = g_strconcat( g_getenv("HOME"),"/", base_directory, "/", ITEM_TABLE, "/", item_id, NULL);

	g_remove(filename);

        return 0;
}
