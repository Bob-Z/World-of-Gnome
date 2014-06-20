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
#include <gio/gio.h>
#include "../common/common.h"
#include <gtk/gtk.h>
#include "imageDB.h"
#include "file.h"
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include "sdl.h"
#include "screen.h"

const char optstring[] = "?i:u:p:l:";
const struct option longopts[] = {
	{ "ip",required_argument,NULL,'i' },
	{ "user",required_argument,NULL,'u' },
	{ "pass",required_argument,NULL,'p' },
	{ "log",required_argument,NULL,'l' },
	{NULL,0,NULL,0}
};

context_t * context;

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

	context_set_username(context,user);

	sdl_init(context);

	imageDB_init();

/*
	if( ! win_login_init(context) ) {
		werr(LOGUSER,"Unable to initialize the login window, abort");
		return 1;
	}

	if( ! win_select_character_init(context) ) {
		werr(LOGUSER,"Unable to initialize the select character window, abort");
		return 1;
	}

	if( ! win_game_init(context) ) {
		werr(LOGUSER,"Unable to initialize the game window, abort");
		return 1;
	}

	win_login_set_entry(ip, user, pass);
*/

	init_log(log);

	/* connect to server */
        if( network_connect(context,ip) ) {
                network_login(context, user, pass);
        } else {
		werr(LOGUSER,"Can't connect to server. Check server IP address. This error may be due to a service outage on server side. Re-try in a few seconds.\n");
		return 0;
        }

	//Run the main loop
	screen_display(context);

	return 0;
}
