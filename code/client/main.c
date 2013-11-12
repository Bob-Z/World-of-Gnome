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
#include <gio/gio.h>
#include "../common/common.h"
#include <gtk/gtk.h>
#include "win_login.h"
#include "win_select_character.h"
#include "win_game.h"
#include "imageDB.h"
#include "file.h"

context_t * context;

/**************************
  default_image_init
**************************/
void default_image_init()
{
	gchar * tmp;
	GdkPixbuf *pixbuf = NULL;

	tmp = g_strconcat( g_getenv("HOME"),"/", base_directory, "/", IMAGE_TABLE, "/", DEFAULT_IMAGE_FILE , NULL);
	if ( g_file_test(tmp,G_FILE_TEST_EXISTS) == TRUE ) {
		g_free(tmp);
		return;
	}

	create_directory(tmp);

	pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB,FALSE,8,1,1);
	gdk_pixbuf_save(pixbuf,tmp,"jpeg",NULL,"quality", "100", NULL);
	g_free(tmp);
}

/**************************
  main
**************************/

int main (int argc, char **argv)
{
	g_type_init();
	g_thread_init(NULL);
	gdk_threads_init();
	gtk_init (&argc, &argv);

        context = context_new();

	default_image_init();

	imageDB_init();

	if( ! win_login_init(context) ) {
		g_warning("Unable to initialize the login window, abort");
		return 1;
	}
	
	if( ! win_select_character_init(context) ) {
		g_warning("Unable to initialize the select character window, abort");
		return 1;
	}

	if( ! win_game_init(context) ) {
		g_warning("Unable to initialize the game window, abort");
		return 1;
	}

	if(argc == 4) {
		win_login_set_entry(argv[1], argv[2], argv[3]);
	}
	
	if(argc == 5) {
		gint log_level = g_ascii_strtoll(argv[4],NULL,10);
		init_log(log_level);
		win_login_set_entry(argv[1], argv[2], argv[3]);
	}
	
	//Run the main loop
	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();

	return 0;
}
