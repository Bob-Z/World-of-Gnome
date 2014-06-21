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
#include <signal.h>
#include <stdlib.h>
#include <npc.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>

const char optstring[] = "?l:";
const struct option longopts[] = {
	{ "log",required_argument,NULL,'l' },
	{NULL,0,NULL,0}
};

void sigint_handler(int sig)
{
	file_dump_all_to_disk();
	printf("Exiting\n");
	exit(0);
}

/**************************
  main
 **************************/

int main (int argc, char **argv)
{
	int opt_ret;
	char * log = NULL;

	while((opt_ret = getopt_long(argc, argv, optstring, longopts, NULL))!=-1) {
		switch(opt_ret) {
		case 'l':
			log = strdup(optarg);;
			break;
		default:
			printf("HELP:\n\n");
			printf("-l --log: Set log level\n");
			exit(0);
		}
	}

	mutex_init();

	//init the main loop
	GMainLoop * mainLoop = NULL;
	mainLoop = g_main_loop_new(NULL,FALSE);

	init_log(log);

	//init non playing character
	init_npc();
	//init network server
	network_init();

	signal(SIGINT,sigint_handler);

	/* Run main loop */
	g_main_loop_run(mainLoop);

	return 0;
}
