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
#include "entry.h"
#include "global.h"
#include "log.h"
#include "syntax.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <unistd.h>

#include <iostream>

using json = nlohmann::json;

client_conf_t clientConf;

/*****************************************************************************/
void client_conf_init()
{
	clientConf.show_tile_type = false;
	clientConf.show_fps = false;
}

/*****************************************************************************/
client_conf_t & client_conf_get()
{
	return clientConf;
}

/*****************************************************************************/
static void parse_client_conf()
{
	int version = 0;

	do
	{
		try
		{
			version = getDataManager().get<int>("", CLIENT_CONF_FILE,
			{ CLIENT_KEY_VERSION });
			LOG_USER("version = " + std::to_string(version));
		} catch (...)
		{
			LOG_USER("Waiting for client configuration");
			usleep(100000);
		}
	} while (version == 0);
	LOG_USER("Client configuration received");

	/*
	 json json;

	 do
	 {
	 try
	 {
	 std::ifstream stream(base_directory + "/" + CLIENT_CONF_FILE);
	 stream >> json;
	 } catch (...)
	 {
	 LOG_USER("Waiting for client configuration");
	 usleep(100000);
	 }
	 } while (json.is_null() == true);
	 */

	//int version = json.at(CLIENT_KEY_VERSION).get<int>();
	//LOG_USER("version = " + std::to_string(version));
	/*
	 */
}

/*****************************************************************************/
void client_conf_read()
{
	parse_client_conf();
}
