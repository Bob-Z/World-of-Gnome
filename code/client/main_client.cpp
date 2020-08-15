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

#include "client_conf.h"
#include "client_server.h"
#include "const.h"
#include "Context/ContextContainer.h"
#include "DataManager/DataManagerClient.h"
#include "file.h"
#include "imageDB.h"
#include "log.h"
#include "lua_client.h"
#include "mutex.h"
#include "network_client.h"
#include "screen.h"
#include "sdl.h"
#include <getopt.h>
#include <SDL2/SDL_mixer.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char optstring[] = "?i:u:p:l:f:F:tPm";
const struct option longopts[] =
{
{ "ip", required_argument, nullptr, 'i' },
{ "user", required_argument, nullptr, 'u' },
{ "pass", required_argument, nullptr, 'p' },
{ "log", required_argument, nullptr, 'l' },
{ "file", required_argument, nullptr, 'f' },
{ "func", required_argument, nullptr, 'F' },
{ "type", no_argument, nullptr, 't' },
{ "fps", no_argument, nullptr, 'P' },
{ "maxfps", no_argument, nullptr, 'm' },
{ nullptr, 0, nullptr, 0 } };

ContextContainer contextContainer;
static DataManagerClient dataManager;

/*****************************************************************************/
ContextContainer & getContextContainer()
{
	return contextContainer;
}

/*****************************************************************************/
DataManagerClient & getDataManager()
{
	return dataManager;
}

/*****************************************************************************/
int main(int argc, char **argv)
{
	int maxfps = false;

	base_directory = std::string(getenv("HOME")) + "/.config/wog/client";

	lua_init();

	client_conf_init();

	std::string ip;
	std::string user;
	std::string pass;
	int opt_ret = 0;
	while ((opt_ret = getopt_long(argc, argv, optstring, longopts, nullptr)) != -1)
	{
		switch (opt_ret)
		{
		case 'i':
			ip = std::string(optarg);
			break;
		case 'u':
			user = std::string(optarg);
			break;
		case 'p':
			pass = std::string(optarg);
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
		case 't':
			client_conf_get().show_tile_type = true;
			break;
		case 'P':
			client_conf_get().show_fps = true;
			break;
		case 'm':
			maxfps = true;
			break;
		default:
			printf("HELP:\n\n");
			printf("-i --ip : Set a server IP\n");
			printf("-u --user: Set a user name\n");
			printf("-p --pass: Set a user password\n");
			printf("-l --log: Set log level\n");
			printf("-f --file: Only display logs from this source file\n");
			printf("-F --func: Only display logs from this function\n");
			printf("-t --type: Show tile type on map\n");
			printf("-P --fps: Show FPS\n");
			printf("-m --maxfps: Display at maximum FPS\n");
			exit(0);
		}
	}

	sdl_init(TITLE_NAME, !maxfps);

	int Mix_flags = MIX_INIT_FLAC | MIX_INIT_MP3 | MIX_INIT_OGG;
	int result;
	if (Mix_flags != (result = Mix_Init(Mix_flags)))
	{
		werr(LOGUSER, "Could not initialize mixer (result: %d).\n", result);
		werr(LOGDESIGNER, "Mix_Init result: %s\n", Mix_GetError());
	}

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);

	Context * context = context_new();

	Connection connection;
	connection.setUserName(user);

	context->setConnection(&connection);

	if (network_connect(connection, ip) == true)
	{
		network_login(connection, user, pass);
	}
	else
	{
		werr(LOGUSER, "Can't connect to server. Check server IP address. This error may be due to a service outage on server side. Re-try in a few seconds.\n");
		return 0;
	}

	while (connection.isConnected() == false)
	{
		usleep(100000);
	}

	client_conf_read();

	//Run the main loop
	screen_display(context);

	SDLNet_Quit();

	return 0;
}
