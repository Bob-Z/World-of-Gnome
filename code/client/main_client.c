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
#include "network_client.h"
#include "imageDB.h"
#include "file.h"
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include "../sdl_item/sdl.h"
#include "screen.h"
#include "option_client.h"
#include <SDL2/SDL_mixer.h>

const char optstring[] = "?i:u:p:l:f:F:tPm";
const struct option longopts[] = {
	{ "ip",required_argument,NULL,'i' },
	{ "user",required_argument,NULL,'u' },
	{ "pass",required_argument,NULL,'p' },
	{ "log",required_argument,NULL,'l' },
	{ "file",required_argument,NULL,'f' },
	{ "func",required_argument,NULL,'F' },
	{ "type",no_argument,NULL,'t' },
	{ "fps",no_argument,NULL,'P' },
	{ "maxfps",no_argument,NULL,'m' },
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
	option_t * option;
	int maxfps = false;

	base_directory = strconcat(getenv("HOME"),"/.config/wog/client",NULL);

	option_init();
	option = option_get();

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
		case 't':
			option->show_tile_type = true;
			break;
		case 'P':
			option->show_fps = true;
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

	common_mutex_init();

	context = context_new();

	context_set_username(context,user);

	sdl_init(TITLE_NAME, &context->render, &context->window, screen_compose, !maxfps);

	int Mix_flags =  MIX_INIT_FLAC |  MIX_INIT_MOD |  MIX_INIT_MP3 |  MIX_INIT_OGG ;
	int result;
	if (Mix_flags != (result = Mix_Init(Mix_flags))) {
		werr(LOGUSER,"Could not initialize mixer (result: %d).\n", result);
		werr(LOGDEV,"Mix_Init result: %s\n", Mix_GetError());
	}

	//Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024);
	Mix_OpenAudio(48000, MIX_DEFAULT_FORMAT, 2, 4096);

	/* connect to server */
	if( network_connect(context,ip) ) {
		network_login(context, user, pass);
	} else {
		werr(LOGUSER,"Can't connect to server. Check server IP address. This error may be due to a service outage on server side. Re-try in a few seconds.\n");
		return 0;
	}

	//Run the main loop
	screen_display(context);

	SDLNet_Quit();

	free(base_directory);

	return 0;
}
