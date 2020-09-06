/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2020 carabobz@gmail.com

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

#include "client_server.h"
#include "DataManager.h"
#include "log.h"
#include "mutex.h"
#include "network_server.h"
#include "npc.h"
#include <dirent.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

const char optstring[] = "?l:f:F:ni:";
const struct option longopts[] =
{
{ "log", required_argument, nullptr, 'l' },
{ "file", required_argument, nullptr, 'f' },
{ "func", required_argument, nullptr, 'F' },
{ "nonpc", no_argument, nullptr, 'n' },
{ "input", required_argument, nullptr, 'i' },
{ nullptr, 0, nullptr, 0 } };

static DataManager dataManager;

/*****************************************************************************/
void sigint_handler(int sig)
{
	printf("Exiting\n");
	exit(0);
}

/*****************************************************************************/
DataManager & getDataManager()
{
	return dataManager;
}

/*****************************************************************************/
int main(int argc, char **argv)
{
	int opt_ret;
	DIR * dir;

	bool npc_allowed = true;

	base_directory = std::string(getenv("HOME")) + "/.config/wog/server";

	while ((opt_ret = getopt_long(argc, argv, optstring, longopts, nullptr)) != -1)
	{
		switch (opt_ret)
		{
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
			npc_allowed = false;
			break;
		case 'i':
			base_directory = std::string(optarg);
			break;
		default:
			printf("HELP:\n\n");
			printf("-l --log: Set log level (user, designer or developer)\n");
			printf("-f --file: Only display logs from this source file\n");
			printf("-F --func: Only display logs from this function\n");
			printf("-n --nonpc: Do not instantiate NPCs\n");
			printf("-i --input: Input data directory (default value: ~/.config/wog/server)\n");
			exit(0);
		}
	}

	dir = opendir(base_directory.c_str());
	if (dir == nullptr)
	{
		werr(LOGUSER, "Cannot find %s directory", base_directory.c_str());
		return -1;
	}
	closedir(dir);

	//init non playing character
	if (npc_allowed == true)
	{
		init_npc();
	}

	signal(SIGINT, sigint_handler);

	//init network server
	network_init();

	return 0;
}
