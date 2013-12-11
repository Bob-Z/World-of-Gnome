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

#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gstdio.h>
#include "../common/common.h"
#include <dirent.h>
#include "imageDB.h"

extern GStaticMutex file_mutex;

/* return 0 if directory was successfully created */
int create_directory(gchar * filename)
{
	gchar * directory = g_strdup(filename);
	gint i;
	for( i = g_utf8_strlen(directory,-1); i > 0; i--) {
		if(directory[i] == '/') {
			directory[i]=0;
			break;
		}
	}

	gboolean res = g_mkdir_with_parents(directory,0777);

	g_free(directory);

	return res;
}


/* return 0 on success */
int file_add(gchar * data,guint32 command_size,gchar ** out_filename)
{
	/* Get the data from the network frame */
	gchar * ptr = data;
	/* First 4 bytes are the size of the file name*/
	if( command_size < sizeof(guint32) ) {
		werr(LOGDEV,"Invalid file received");
		return 1;
	}
	guint32 file_name_size = *((guint32 *)ptr);

	/* Following bytes are the file name, relative to the application base directory ( $HOME/.config/wog/client/ ) */
	ptr += sizeof(guint32);
	gchar * file_name = NULL;
	file_name = g_memdup(ptr,file_name_size);
	if( file_name == NULL ) {
		werr(LOGDEV,"Unable to allocate %d bytes for file name",file_name_size);
		return 1;
	}

	/* Next is a guint32 representing the size of the file's data */
	ptr += file_name_size;
	guint32 file_data_size = *((guint32 *)ptr);

	/* Finally are the data bytes */
	ptr += sizeof(guint32);

	/* Write the data to disk */
	gchar * full_name = NULL;
	full_name = g_strconcat( g_getenv("HOME"),"/", base_directory, "/",file_name, NULL);

	if( create_directory(full_name)) {
		g_free(full_name);
		werr(LOGDEV,"Can't create directory for %s", full_name);
		return 1;
	}
	g_static_mutex_lock(&file_mutex);
	gboolean res = file_set_contents(full_name,ptr,file_data_size,NULL);
	g_static_mutex_unlock(&file_mutex);
	if( res == FALSE ) {
		werr(LOGDEV,"Error writing file %s with size %d",full_name, file_data_size);
		return 1;
	}

	wlog(LOGDEBUG,"write file %s",full_name);

	/* Update the image DB */
	GdkPixbufAnimation * pixbuf = gdk_pixbuf_animation_new_from_file(full_name,NULL);
	if( pixbuf != NULL ) {
		/* Check for this file in the imageDB */
		GtkWidget * image = imageDB_check_widget(file_name);
		if( image != NULL ) {
			/* The image exists, update the pixbuf */
			gdk_threads_enter();
			gtk_image_set_from_animation(GTK_IMAGE(image),pixbuf);
			gdk_threads_leave();
		}
		g_object_unref(pixbuf);
	}

	if( out_filename != NULL ) {
		*out_filename = file_name;
	} else {
		g_free(file_name);
	}

	g_free(full_name);
	return 0;
}

/* Remove character file to be sure they are always downloaded at start-up time */
/* called by client */
void file_clean(context_t * context)
{
	gchar * filename;

	filename = g_strconcat( g_getenv("HOME"),"/", base_directory, "/",CHARACTER_TABLE,"/",context->id, NULL);
	g_unlink(filename);
	g_free( filename);
}
