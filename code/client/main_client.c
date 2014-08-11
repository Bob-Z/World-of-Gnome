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
#include "imageDB.h"
#include "file.h"
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include "../sdl_item/sdl.h"
#include "screen.h"

const char optstring[] = "?i:u:p:l:f:F:";
const struct option longopts[] = {
	{ "ip",required_argument,NULL,'i' },
	{ "user",required_argument,NULL,'u' },
	{ "pass",required_argument,NULL,'p' },
	{ "log",required_argument,NULL,'l' },
	{ "file",required_argument,NULL,'f' },
	{ "func",required_argument,NULL,'F' },
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
	
	base_directory = strconcat(getenv("HOME"),"/.config/wog/client",NULL);

	while((opt_ret = getopt_long(argc, argv, optstring, longopts, NULL))!=-1) {
		switch(opt_ret) {
		case 'i':
			ip = strdup(optarg);
			break;
		case 'u':
			user = strdup(optarg);
			break;
		case 'p':
			pass = strdup(optarg);
			break;
		case 'l':
			log_set_level(optarg);
			break;
		case 'f':
			log_add_file_filter(optarg);
			break;
		case 'F':
			log_add_func_filter(optarg);
			break;
		default:
			printf("HELP:\n\n");
			printf("-i --ip : Set a server IP\n");
			printf("-u --user: Set a user name\n");
			printf("-p --pass: Set a user password\n");
			printf("-l --log: Set log level\n");
			printf("-f --file: Only display logs from this source file\n");
			printf("-F --func: Only display logs from this function\n");
			exit(0);
		}
	}

	common_mutex_init();

	context = context_new();

	context_set_username(context,user);

	sdl_init(&context->render, &context->window, screen_compose);

	/* connect to server */
        if( network_connect(context,ip) ) {
                network_login(context, user, pass);
        } else {
		werr(LOGUSER,"Can't connect to server. Check server IP address. This error may be due to a service outage on server side. Re-try in a few seconds.\n");
		return 0;
        }

	//Run the main loop
	screen_display(context);

	free(base_directory);
	
	return 0;
}
