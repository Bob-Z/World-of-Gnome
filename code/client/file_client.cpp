/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2019 carabobz@gmail.com

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
#include "common.h"
#include "const.h"
#include "Context.h"
#include "entry.h"
#include "file.h"
#include "syntax.h"
#include "log.h"
#include <cstdio>
#include <string>

#include "imageDB.h"
#include "option_client.h"
#include "screen.h"

/*************************************
 return 0 on success
 **************************************/
int file_add(Context * context, const std::string & name, const std::string & data)
{
	const std::string temp_name = name + APP_NAME + "tmp";
	const std::string temp_path = base_directory + "/" + temp_name;

	file_create_directory(temp_path);

	if (file_set_contents(temp_name.c_str(), data.c_str(), data.size()) == RET_NOK)
	{
		werr(LOGDESIGNER, "Error writing file %s with size %d", temp_name.c_str(), data.size());
		return -1;
	}

	const std::string file_path = base_directory + "/" + name;

	if (rename(temp_path.c_str(), file_path.c_str()) == -1)
	{
		wlog(LOGDEVELOPER, "Error renaming file %s to %s", temp_path.c_str(), file_path.c_str());
		return RET_NOK;
	}

	wlog(LOGDEVELOPER, "write file %s", file_path.c_str());

	// Update the entry DB
	entry_remove(name.c_str());
	// Update the image DB
	image_DB_remove(name.c_str());
	// Update options if needed
	option_read_client_conf();
	// Make sure the new file is drawn (if needed)
	screen_compose();

	return 0;
}

/*********************************************************************************
 Remove character file to be sure they are always down loaded at start-up time
 **********************************************************************************/
void file_clean(Context * context)
{
	file_delete(CHARACTER_TABLE, context->getId());
}

/***************************************************
 Request a file from network
 ****************************************************/
void file_request_from_network(Context * context, const char * table, const char * file_name)
{
	const std::string table_path = std::string(table) + "/" + std::string(file_name);

	file_lock(table_path.c_str());
	file_update(context, table_path.c_str());
	file_unlock(table_path.c_str());

	return;
}
