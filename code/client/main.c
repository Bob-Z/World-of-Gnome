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
#include <getopt.h>
#include <string.h>
#include <stdlib.h>

const char optstring[] = "?i:u:p:l:";
const struct option longopts[] =
        {{ "ip",required_argument,NULL,'i' },
        { "user",required_argument,NULL,'u' },
        { "pass",required_argument,NULL,'p' },
        { "log",required_argument,NULL,'l' },
        {NULL,0,NULL,0}};

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
	int opt_ret;
	char * ip = NULL;
	char * user = NULL;
	char * pass = NULL;
	char * log = NULL;


	g_type_init();
	g_thread_init(NULL);
	gdk_threads_init();
	gtk_init (&argc, &argv);

	while((opt_ret = getopt_long(argc, argv, optstring, longopts, NULL))!=-1) {
		switch(opt_ret) {
			case 'i':
				ip = strdup(optarg);;
				break;
			case 'u':
				user = strdup(optarg);;
				break;
			case 'p':
				pass = strdup(optarg);;
				break;
			case 'l':
				log = strdup(optarg);;
				break;
			default:
				printf("HELP:\n\n");
				printf("-i --ip : Set a server IP\n");
				printf("-u --user: Set a user name\n");
				printf("-p --pass: Set a user password\n");
				printf("-l --log: Set log level\n");
				exit(0);
		}
	}

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

	win_login_set_entry(ip, user, pass);

	init_log(log);
	
	//Run the main loop
	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();

	return 0;
}
