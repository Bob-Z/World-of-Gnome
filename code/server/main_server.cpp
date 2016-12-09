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
#include "network_server.h"
#include "npc.h"
#include <signal.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

static int noNPC = 0;

const char optstring[] = "?l:f:F:ni:";
const struct option longopts[] = {
	{ "log",required_argument,NULL,'l' },
	{ "file",required_argument,NULL,'f' },
	{ "func",required_argument,NULL,'F' },
	{ "nonpc",no_argument,NULL,'n' },
	{ "input",required_argument,NULL,'i' },
	{NULL,0,NULL,0}
};

void sigint_handler(int sig)
{
	printf("Exiting\n");
	exit(0);
}

/**************************
  main
 **************************/
int main (int argc, char **argv)
{
	int opt_ret;
	DIR * dir;

	while((opt_ret = getopt_long(argc, argv, optstring, longopts, NULL))!=-1) {
		switch(opt_ret) {
		case 'l':
			log_set_level(optarg);
			break;
		case 'f':
			log_add_file_filter(optarg);
			break;
		case 'F':
			log_add_func_filter(optarg);
			break;
		case 'n':
			noNPC = 1;
			break;
		case 'i':
			base_directory = strdup(optarg);
			break;
		default:
			printf("HELP:\n\n");
			printf("-l --log: Set log level\n");
			printf("-f --file: Only display logs from this source file\n");
			printf("-F --func: Only display logs from this function\n");
			printf("-n --nonpc: Do not instantiate NPCs\n");
			printf("-i --input: Input data directory (default value: ~/.config/wog/server)\n");
			exit(0);
		}
	}

	if(base_directory == NULL) {
		base_directory = strconcat(getenv("HOME"),"/.config/wog/server",NULL);
	}

	dir = opendir(base_directory);
	if(dir == NULL) {
		werr(LOGUSER,"Cannot find %s directory",base_directory);
		return -1;
	}
	closedir(dir);

	common_mutex_init();

	//init non playing character
	if( ! noNPC ) {
		init_npc();
	}

	signal(SIGINT,sigint_handler);

	//init network server
	network_init();

	free(base_directory);

	return 0;
}
